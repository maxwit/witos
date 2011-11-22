#include <mmc/mmc.h>

#define SD_IO_SEND_OP_COND 5
#define SD_IO_RW_DIRECT    52

int mmc_send_io_op_cond(struct mmc_host *host, __u32 ocr, __u32 *rocr)
{
	struct mmc_command cmd;
	int ret = 0;

	cmd.index = SD_IO_SEND_OP_COND;
	cmd.arg = ocr;
	cmd.resp = R4;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	*rocr = host->resp[0] & 0xffffff;

	return ret;
}

int mmc_io_rw_direct(struct mmc_host *host, int write, unsigned fn,
	unsigned addr, __u8 in, __u8* out)
{
	struct mmc_command cmd;
	int ret = 0;

	cmd.arg = write ? 0x80000000 : 0x00000000;
	cmd.arg |= fn << 28;
	cmd.arg |= (write && out) ? 0x08000000 : 0x00000000;
	cmd.arg |= addr << 9;
	cmd.arg |= in;

	cmd.index = SD_IO_RW_DIRECT;
	cmd.resp = R5;

	ret = host->send_cmd(host, cmd.index, cmd.arg, cmd.resp);

	if (NULL != out)
		*out = host->resp[0];

	return ret;
}

int mmc_io_rw_extended(struct mmc_host *host, int write, unsigned fn,
	unsigned addr, int incr_addr, __u8 *buf, unsigned blocks, unsigned blksz)
{
	int ret = 0;

	return ret;
}
