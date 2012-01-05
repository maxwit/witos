#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <mmc/mmc.h>
#include <mmc/mmc_ops.h>
#include "sdio_ops.h"
#include "sdio.h"

static int sdio_read_cis(struct mmc_host *host)
{
	int ret = 0;
	unsigned int i, ptr = 0;
	unsigned int vendor, device;
	struct sdio_func_tuple *this;

	for (i = 0; i < 3; i++) {
		unsigned char x, fn = 0;

		ret = mmc_io_rw_direct(host, 0, 0, SDIO_FBR_BASE(fn) + SDIO_FBR_CIS + i, 0, &x);
		if (ret)
			return ret;

		ptr |= x << (i * 8);
	}

	DPRINT("ptr = 0x%x\n", ptr);

	do {
		unsigned char tpl_code, tpl_link;

		ret = mmc_io_rw_direct(host, 0, 0, ptr++, 0, &tpl_code);
		if (ret)
			break;

		/* 0xff means we're done */
		if (tpl_code == 0xff)
			break;

		/* null entries have no link field or data */
		if (tpl_code == 0x00)
			continue;

		ret = mmc_io_rw_direct(host, 0, 0, ptr++, 0, &tpl_link);
		if (ret)
			break;

		/* a size of 0xff also means we're done */
		if (tpl_link == 0xff)
			break;

		this = malloc(sizeof(*this) + tpl_link);
		if (!this)
			return -ENOMEM;

		for (i = 0; i < tpl_link; i++) {
			ret = mmc_io_rw_direct(host, 0, 0, ptr + i, 0, &this->data[i]);
			if (ret)
				break;
		}

		if (ret) {
			free(this);
			break;
		}

		ptr += tpl_link;

		if (tpl_code == 0x20) {

			if (tpl_link < 4) {
				printf("bad CIS tuple len = %d\n", tpl_link);
				return  -EINVAL;
			}

			vendor = this->data[0] | (this->data[1] << 8);

			device = this->data[2] | (this->data[3] << 8);

			printf("vendor = 0x%x, device = 0x%x\n", vendor, device);
		}
	} while (!ret);

	return ret;
}

static int sdio_read_cccr(struct mmc_host *host)
{
	int ret;
	int cccr_vsn;
	unsigned char data;
	struct mmc_card *card = &host->card;

	memset(&card->cccr, 0, sizeof(struct sdio_cccr));

	ret = mmc_io_rw_direct(host, 0, 0, SDIO_CCCR_CCCR, 0, &data);
	if (ret)
		goto out;

	cccr_vsn = data & 0x0f;

	if (cccr_vsn > SDIO_CCCR_REV_1_20) {
		printf("%s: unrecognised CCCR structure version %d\n", __func__, cccr_vsn);
		return -EINVAL;
	}

	card->cccr.sdio_vsn = (data & 0xf0) >> 4;

	ret = mmc_io_rw_direct(host, 0, 0, SDIO_CCCR_CAPS, 0, &data);
	if (ret)
		goto out;

	if (data & SDIO_CCCR_CAP_SMB)
		card->cccr.multi_block = 1;
	if (data & SDIO_CCCR_CAP_LSC)
		card->cccr.low_speed = 1;
	if (data & SDIO_CCCR_CAP_4BLS)
		card->cccr.wide_bus = 1;

	if (cccr_vsn >= SDIO_CCCR_REV_1_10) {
		ret = mmc_io_rw_direct(host, 0, 0, SDIO_CCCR_POWER, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_POWER_SMPC)
			card->cccr.high_power = 1;
	}

	if (cccr_vsn >= SDIO_CCCR_REV_1_20) {
		ret = mmc_io_rw_direct(host, 0, 0, SDIO_CCCR_SPEED, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_SPEED_SHS)
			card->cccr.high_speed = 1;
	}

out:
	return ret;
}

static int mmc_sdio_init_card(struct mmc_host *host)
{
	int i = 0, ret = 0;
	__u32  rocr;

	ret = mmc_io_rw_direct(host, 1, 0, 6, 8, NULL);
	for (i = 0; i < 10; i++) {
		ret = mmc_send_io_op_cond(host, 0, &rocr);
		if (!(host->resp[0] & (7 << 28)))
			return -1;

		ret = mmc_send_io_op_cond(host, rocr, &rocr);

		if (host->resp[0] & 0x80000000)
			break;
	}

	ret = mmc_set_relative_addr(host);

	DPRINT("rca = %d\n", host->card.rca);

	ret = mmc_select_card(host);

	ret = sdio_read_cccr(host);

	ret = sdio_read_cis(host);

	return ret;
}

int sdio_register(struct mmc_host *host)
{
	mmc_sdio_init_card(host);

	return 0;
}
