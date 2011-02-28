#include <g-bios.h>
#include <mmc/mmc.h>
#include "mmc_ops.h"

#if 0
#define BLKNR 1
#define BSIZE 512

u32 dataddr = 0;

struct mmc_command mmc_cmd[] =
{
	{MMC_CMD0, 0, NONE}, {MMC_CMD1, 0, REV}, {MMC_CMD2, 0, R2}, {MMC_CMD3, 0, R6}, {MMC_CMD4, 0, NONE},
	{MMC_CMD5, 0, REV}, {MMC_CMD6, 0, R1}, {MMC_CMD7, 0, R1b}, {MMC_CMD8, 0, R7}, {MMC_CMD9, 0, R2},
	{MMC_CMD10, 0, R2}, {MMC_CMD11, 0, REV}, {MMC_CMD12, 0, R1b}, {MMC_CMD13, 0, R1}, {MMC_CMD14, 0, REV},
	{MMC_CMD15, 0, NONE}, {MMC_CMD16, 0, R1}, {17, 0, R1}, {MMC_CMD18, 0, R1}, {MMC_CMD19, 0, REV},
	{MMC_CMD20, 0, REV}, {MMC_CMD21, 0, REV}, {MMC_CMD22, 0, REV}, {MMC_CMD23, 0, REV}, {MMC_CMD24, 0, R1},
	{MMC_CMD25, 0, R1}, {MMC_CMD26, 0, REV}, {MMC_CMD27, 0, R1}, {MMC_CMD28, 0, R1b}, {MMC_CMD29, 0, R1b},
	{MMC_CMD30, 0, R1}, {MMC_CMD31, 0, REV}, {MMC_CMD32, 0, R1}, {MMC_CMD33, 0, R1}, {MMC_CMD34, 0, REV},
	{MMC_CMD35, 0, REV}, {MMC_CMD36, 0, REV}, {MMC_CMD37, 0, REV}, {MMC_CMD38, 0, R1b}, {MMC_CMD39, 0, REV},
	{MMC_CMD40, 0, REV}, {MMC_CMD41, 0, R3}, {MMC_CMD42, 0, R1}, {MMC_CMD43, 0, REV}, {MMC_CMD44, 0, REV},
	{MMC_CMD45, 0, REV}, {MMC_CMD46, 0, REV}, {MMC_CMD47, 0, REV}, {MMC_CMD48, 0, REV}, {MMC_CMD49, 0 ,REV},
	{MMC_CMD50, 0, REV}, {MMC_CMD51, 0, REV}, {MMC_CMD52, 0, REV}, {MMC_CMD53, 0, REV}, {MMC_CMD54, 0, REV},
	{MMC_CMD55, 0, R1}, {MMC_CMD56, 0, R1}, {MMC_CMD57, 0, REV}, {MMC_CMD58, 0, REV}, {MMC_CMD59, 0, REV},
	{MMC_CMD60, 0, REV}, {MMC_CMD61, 0, REV}, {MMC_CMD62, 0, REV}, {MMC_CMD63, 0, REV},
};


static int mmc_send_cmd(struct mmc_host *host, u32 cmd)
{
	u32 arg = mmc_cmd[cmd].arg;
	RESP resp;

#if 1
	switch (cmd)
	{
	case MMC_CMD4:
		arg = host->info.dsr << 16;
	case MMC_CMD6:
		arg = 2;
		break;
	case MMC_CMD8:
		arg = 0x1aa;
		break;

	case MMC_CMD41:
		arg = 0xff8000;
		break;

	case MMC_CMD7:
	case MMC_CMD9:
	case MMC_CMD10:
	case MMC_CMD13:
	case MMC_CMD15:
	case MMC_CMD55:
		arg = host->info.rca << 16;
		break;

	case MMC_CMD16:
		arg = 512;
		break;

	case MMC_CMD17:
	case MMC_CMD18:
	case MMC_CMD24:
	case MMC_CMD25:
	case MMC_CMD28:
	case MMC_CMD29:
	case MMC_CMD30:
	case MMC_CMD32:
	case MMC_CMD33:
		arg = dataddr;

	default:
		break;
	}
#endif
	resp  = mmc_cmd[cmd].resp;

	return host->send_cmd(host, cmd, arg, resp);
}


static int mmc_write_blk(struct mmc_host *host, char *buf)
{
		mmc_send_cmd(host, MMC_CMD24);
		host->write_data(host, buf);

		udelay(1000);

	return 0;
}


static int mmc_read_blk(struct mmc_host *host, char *buf)
{
	mmc_send_cmd(host, MMC_CMD17);
	host->read_data(host, buf);

	udelay(1000);

	if (1)
	{
		int i;

		for (i = 0; i < BSIZE / 4; i++)
		{
			printf("%08x", ((u32*)buf)[i]);

			if ((i + 1)% 8)
			{
				printf(" ");
			}
			else
			{
				printf("\n");
			}
		}
	}

	return 0;

}


static int mmc_erase_blk(struct mmc_host *host)
{
	mmc_send_cmd(host, MMC_CMD32);
	mmc_send_cmd(host, MMC_CMD33);
	mmc_send_cmd(host, MMC_CMD38);

	while (1)
	{
		mmc_send_cmd(host, MMC_CMD13);
		if (host->resp[0] & 0x100)
			break;
	}

	return 0;
}
#endif

static int mmc_decode_cid(struct mmc_host *host)
{
	char cid[6];

	cid[0] = (char)(host->resp[3]) & 0xff;
	cid[1] = host->resp[2] >> 24;
	cid[2] = (host->resp[2] >> 16) & 0xff;
	cid[3] = (host->resp[2] >> 8) & 0xff;
	cid[4] = host->resp[2] & 0xff;
	cid[5] = '\0';

	if (cid[0])
		printf("CID = %s\n", cid);
	else
	{
		printf("No SD card found!\n");
		return -1;
	}

	return 0;

}

static int mmc_sd_init_card(struct mmc_host *host)
{
	int ret = 0;
	struct mmc_card *card = &host->info;

	/*cmd0*/
	ret = mmc_go_idle(host);
	if (ret)
		goto out;
	/*cmd8*/
	ret = mmc_send_if_cond(host, 0x1aa);
	if (ret)
		goto out;

	/*acmd41*/
	ret = mmc_send_app_op_cond(host, 0xff8000, NULL);
	if (ret)
		goto out;

	/*cmd2*/
	ret = mmc_all_send_cid(host, card->raw_cid);
	if (ret)
		goto out;

	ret = mmc_decode_cid(host);
	if (ret)
		goto out;

	/*cmd3*/
	ret = mmc_set_relative_addr(host);
	if (ret)
		goto out;

	/*cmd9*/
	ret = mmc_send_csd(host, card->raw_csd);
	if (ret)
		goto out;

	ret = mmc_select_card(host);
	if (ret)
		goto out;

	mmc_app_set_bus_width(host,SD_BUS_WIDTH_4);

	host->set_hclk();

out:
	return ret;
}

int mmc_register(struct mmc_host *host)
{

	if (!host || !host->send_cmd)
		return -EINVAL;

	mmc_sd_init_card(host);

#if 0
	dataddr = 0;
	mmc_erase_blk(host);

	mmc_read_blk(host, buf);

	memset(buf, 0xa, BLKNR * BSIZE);
	mmc_write_blk(host, buf);

	mmc_read_blk(host, buf);

#endif

	return 0;
}

