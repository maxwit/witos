#include <init.h>
#include <stdio.h>
#include <delay.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <mmc/mmc.h>
#include <mmc/mmc_ops.h>

int mmc_go_idle(struct mmc_host *host)
{
	struct mmc_command cmd;
	int ret = 0;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = MMC_GO_IDLE_STATE;
	cmd.arg = 0;
	cmd.resp = NONE;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	return ret;
}

int mmc_all_send_cid(struct mmc_host *host, __u32 *cid)
{
	struct mmc_command cmd;
	int ret = 0;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = MMC_ALL_SEND_CID;
	cmd.arg = 0;
	cmd.resp = R2;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	memcpy(cid, host->resp, sizeof(__u32) * 4);

	return ret;
}

int mmc_set_relative_addr(struct mmc_host *host)
{
	struct mmc_command cmd;
	int ret = 0;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = MMC_SET_RELATIVE_ADDR;
	cmd.arg = 0;
	cmd.resp = R6;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	host->card.rca = host->resp[0] >> 16;

	return ret;
}

int mmc_select_card(struct mmc_host *host)
{
	struct mmc_command cmd;
	int ret = 0;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = MMC_SELECT_CARD;
	cmd.arg = host->card.rca << 16;
	cmd.resp = R1b;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	return ret;
}

int mmc_send_if_cond(struct mmc_host *host, __u32 ocr)
{
	struct mmc_command cmd;
	int ret = 0;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = SD_SEND_IF_COND;
	cmd.arg = ocr;
	cmd.resp = R7;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	return ret;
}

static int mmc_app_cmd(struct mmc_host *host)
{
	int ret;
	struct mmc_command cmd;

	assert(host);

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index= MMC_APP_CMD;
	cmd.arg = host->card.rca << 16;
	cmd.resp = R1;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	return ret;
}

int mmc_switch_width(struct mmc_host *host)
{
	struct mmc_command cmd;
	int ret = 0;

	ret = mmc_app_cmd(host);
	if (ret)
		return ret;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = MMC_SWITCH;
	cmd.arg = 0x2;
	cmd.resp = R1;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	return ret;
}

int mmc_send_app_op_cond(struct mmc_host *host, __u32 ocr, __u32 *rocr)
{
	int i = 0, ret = 0;
	struct mmc_command cmd;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = SD_APP_OP_COND;
	cmd.arg  = ocr;
	cmd.resp = R3;

	for (i = 0; i < MMC_CMD_RETRIES; i++) {
		ret = mmc_app_cmd(host);

		udelay(100);

		ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

		if (host->resp[0] & 0x80000000)
			break;

		ret = -ETIMEDOUT;

		udelay(100);
	}

	if (rocr)
		*rocr = host->resp[0];

	return ret;
}

int mmc_send_csd(struct mmc_host *host, __u32 *csd)
{
	struct mmc_command cmd;
	int ret = 0;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = MMC_SEND_CSD;
	cmd.arg = host->card.rca << 16;
	cmd.resp = R2;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	memcpy(csd, host->resp, sizeof(__u32) * 4);

	return ret;
}

int mmc_app_set_bus_width(struct mmc_host *host, int width)
{
	int ret = 0;
	struct mmc_command cmd;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = SD_APP_SET_BUS_WIDTH;
	cmd.arg  = width;
	cmd.resp = R1;

	ret = mmc_app_cmd(host);

	udelay(500);

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	return ret;
}

int mmc_set_block_len(struct mmc_host *host, int len)
{
	int ret = 0;
	struct mmc_command cmd;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.index = MMC_SET_BLOCKLEN;
	cmd.arg  = len;
	cmd.resp = R1;

	udelay(500);

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	return ret;
}
