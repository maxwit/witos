#include <mmc/mmc.h>
#include <mmc/mmc_ops.h>
#include <stdio.h>
#include <string.h>
#include <loader.h>

#define BLKNR 1
#define MMC_BLK_SIZE 512
#define GBH_LOAD_SIZE     MB(2)

static struct mmc_host* g_mmc_host;

int mmc_read_blk(struct mmc_host *host, __u8 *buf, int start)
{
	int ret = 0;

	ret = host->send_cmd(host, MMC_READ_SINGLE_BLOCK, start, R1);
	if (ret < 0)
		return ret;

	ret = host->read_data(host, buf);
	if (ret < 0)
		return ret;

	int i;

	for (i = 0; i < MMC_BLK_SIZE / 4; i++)
	{
		printf("%x", ((__u32*)buf)[i]);

		if ((i + 1)% 8)
		{
			printf(" ");
		}
		else
		{
			printf("\n");
		}
	}


	return 0;
}

int mmc_write_blk(struct mmc_host *host, const __u8 *buf, int start)
{
	int ret = 0;

	ret = host->send_cmd(host, MMC_WRITE_BLOCK, start, R1);
	if (ret < 0)
		return ret;

	ret = host->write_data(host, buf);
	if (ret < 0)
		return ret;

	udelay(1000);

	return 0;
}

int mmc_decode_cid(struct mmc_host *host)
{
	char *name = host->card.card_name;

	name[0] = (char)(host->resp[3]) & 0xff;
	name[1] = host->resp[2] >> 24;
	name[2] = (host->resp[2] >> 16) & 0xff;
	name[3] = (host->resp[2] >> 8) & 0xff;
	name[4] = host->resp[2] & 0xff;
	name[5] = '\0';

	if (!name[0])
		return -1;

	return 0;
}




int mmc_sd_detect_card(struct mmc_host *host)
{
	int ret = 0, i;
	__u32 val;
	struct mmc_card *card;

	if (!host)
		return -ENODEV;

	card = &host->card;
	card->host = host;

	if (host->set_lclk)
		host->set_lclk();

	/*cmd0*/
	ret = host->send_cmd(host, MMC_GO_IDLE_STATE, 0, NONE);
	if (ret)
		goto out;

	/*cmd8*/
	// ret = mmc_send_if_cond(host, 0x1AA);
	ret = host->send_cmd(host, SD_SEND_IF_COND, 0x1AA, R7);

	val = (ret == 0) ? 0x40ff8000 : 0x00ff8000;

	/*acmd41*/
	// ret = mmc_send_app_op_cond(host, val, NULL);

	for (i = 0; i < MMC_CMD_RETRIES; i++)
	{
		ret = host->send_cmd(host, MMC_APP_CMD, 0, R1);

		ret = host->send_cmd(host, SD_APP_OP_COND, val, R3);

		if (host->resp[0] & 0x80000000)
			break;

		ret = -ETIMEDOUT;

		udelay(100);
	}

	if (ret)
		goto out;

	/*cmd2*/
	// ret = mmc_all_send_cid(host, card->raw_cid);
	ret = host->send_cmd(host, MMC_ALL_SEND_CID, 0, R2);
	if (ret)
		goto out;

	// memcpy(host->card.raw_cid, host->resp, sizeof(__u32) * 4);

	ret = mmc_decode_cid(host);
	if (ret)
		return ret;

	return 0;
out:
	return ret;
}

int mmc_register(struct mmc_host *host)
{
	int ret;

	g_mmc_host = host;

	ret = mmc_sd_detect_card(host);
	if (ret < 0)
		printf("No SD Found!\n");
	else
		printf("Card = %s\n", host->card.card_name);

	/* cmd3*/
	ret = host->send_cmd(host, MMC_SET_RELATIVE_ADDR, 0, R6);
	if (ret)
		goto out;

	host->card.rca = host->resp[0] >> 16;

#if 0
	/*cmd9*/
	ret = host->send_cmd(host, MMC_SEND_CSD, host->card.rca << 16, R2);
	if (ret)
		goto out;
#endif
	// ret = mmc_select_card(host);
	ret = host->send_cmd(host, MMC_SELECT_CARD, host->card.rca << 16, R1b);
	if (ret)
		goto out;

	if (host->set_hclk)
		host->set_hclk();
	else
	{
		printf("Host not support to switch high frequency!\n");
	}

	printf("Init Sucess!\n");
	return 0;


	//mmc_app_set_bus_width(host,SD_BUS_WIDTH_4);

	//ret = mmc_set_block_len(host, MMC_BLK_SIZE);


out:
	return ret;
}

static int mmc_loader(struct loader_opt *opt)
{
	mmc_init();
	int i;
	int byte_offset = 0;
	opt->load_size = GBH_LOAD_SIZE;
	int load_size = opt->load_size;

	ALIGN_UP(load_size, MMC_BLK_SIZE);

	for (i = 0; i < load_size / MMC_BLK_SIZE; i++, byte_offset += MMC_BLK_SIZE)
	{
		mmc_read_blk(g_mmc_host, opt->load_addr + byte_offset, byte_offset);
	}

	printf("Loader Success!\n");

	return 0;
}

REGISTER_LOADER(m, mmc_loader, "MMC");

