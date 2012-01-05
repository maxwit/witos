#include <mmc/mmc.h>

#define S3C24XX_GPECON  0x56000040

#define CMDSTAR 0x140

//#define CONF_DEBUG

static void s3c2440_mmc_set_hclk(void);
static int s3c2440_send_cmd(struct mmc_host *mmc, __u32 index, __u32 arg, RESP resp);
static int s3c2440_read_data(struct mmc_host *mmc, void *buf);
static int s3c2440_write_data(struct mmc_host *mmc, const void *buf);

static struct mmc_host s3c2440_mmc =
{
	.send_cmd   = s3c2440_send_cmd,
	.set_hclk   = s3c2440_mmc_set_hclk,
	.read_data  = s3c2440_read_data,
	.write_data = s3c2440_write_data,
};

void  s3c2440_mmc_set_hclk(void)
{
	writel(VA(S3C24X0_SDIBASE + SDI_CON), 0);
	writel(VA(S3C24X0_SDIBASE + SDI_PRE), 1);
	writel(VA(S3C24X0_SDIBASE + SDI_CON), 1 << 4 | 1);

	udelay(1000);
}

int s3c2440_write_data(struct mmc_host *mmc, const void *buf)
{
	int i;
	__u32 val;

	for (i = 0; i < 512 / 4; ) {

		val = readl(VA(S3C24X0_SDIBASE + SDIFSTA));
		//printf("FIFO Status = 0x%x\n", val);

		if (val & 1 << 13) {
			writel(VA(S3C24X0_SDIBASE + 0x40), ((__u32 *)buf)[i]);
			i++;
		}
	}

#if 1
	for (i = 0; i < 10; i++) {
		val = readl(VA(S3C24X0_SDIBASE + SDIDATSTA));
		//printf("Data Status = 0x%x count = 0x%x\n", val, readl(VA(S3C24X0_SDIBASE + SDIDATCNT)));
		udelay(1000);
	}
#else
	do {
		val = readl(VA(S3C24X0_SDIBASE + SDIDATSTA));
		printf("Data Status = 0x%x count = 0x%x\n", val, readl(VA(S3C24X0_SDIBASE + SDIDATCNT)));
	}while (val & 0x2);
#endif

	writel(VA(S3C24X0_SDIBASE + SDIDATSTA), val);

	return 0;
}

int s3c2440_read_data(struct mmc_host *mmc, void *buf)
{
	int i = 0;
	__u32 val;

	while (1) {
		val = readl(VA(S3C24X0_SDIBASE + SDIFSTA));
		//printf("FIFO Status = 0x%x\n", val);

		if (val & 1 << 12) {
			((__u32 *)buf)[i] = readl(VA(S3C24X0_SDIBASE + 0x40));
			i++;
		} else
			break;
	}

	return i;
}

int s3c2440_send_cmd(struct mmc_host *mmc, __u32 index, __u32 arg, RESP resp)
{
	__u32 cval, val;

	cval   = index | CMDSTAR;

	switch (resp) {
	case R1:
	case R1b:
	case R3:
	case R6:
	case R7:
		cval |= 1 << 9;
		break;
	case R2:
		cval |= 3 << 9;
		break;

	case REV:
	case NONE:
	default:
		break;
	}

	if (index == MMC_READ_SINGLE_BLOCK)
		writel(VA(S3C24X0_SDIBASE + SDIDATCON), 2 << 22 | 7 << 18 | 3 << 16 | 1 << 14 | 2 << 12 | 0x100);

	if (MMC_WRITE_BLOCK == index) {

		val = readl(VA(S3C24X0_SDIBASE + SDIFSTA));
		writel(VA(S3C24X0_SDIBASE + SDIDATCON),2 << 22 | 7 << 18 | 3 << 16 | 1 << 14 | 3 << 12 | 0x100);
		cval |= 1 << 11;
	}

	writel(VA(S3C24X0_SDIBASE + SDI_CMDARG), arg);
	writel(VA(S3C24X0_SDIBASE + SDI_CMDCON), cval);

	while (1) {
		val = readl(VA(S3C24X0_SDIBASE + SDI_CMDSTA));
		udelay(1000);

		if (NONE == resp) {
			if (val & (1 << 11))
				break;
		} else {
			if(val & (3 << 9))
				break;
		}
	}

	if (val & (1 << 10)) {
	//	printf("cmd%d time out! status = 0x%x\n", index, val);
		return -1;
	}

	if ((MMC_SEND_CID== index) || (MMC_SEND_CSD == index) || (MMC_READ_DAT_UNTIL_STOP == index)) {
		mmc->resp[3] = readl(VA(S3C24X0_SDIBASE + SD_RESP0));
		mmc->resp[2] = readl(VA(S3C24X0_SDIBASE + SD_RESP1));
		mmc->resp[1] = readl(VA(S3C24X0_SDIBASE + SD_RESP2));
		mmc->resp[0] = readl(VA(S3C24X0_SDIBASE + SD_RESP3));
	} else {
		mmc->resp[0] = readl(VA(S3C24X0_SDIBASE + SD_RESP0));
		mmc->resp[1] = readl(VA(S3C24X0_SDIBASE + SD_RESP1));
		mmc->resp[2] = readl(VA(S3C24X0_SDIBASE + SD_RESP2));
		mmc->resp[3] = readl(VA(S3C24X0_SDIBASE + SD_RESP3));
	}
#ifdef CONF_DEBUG
	printf("cmd%d: arg = 0x%x val =0x%x\nresp[0]=0x%8x\nresp[1]=0x%8x\nresp[2]=0x%8x\nresp[3]=0x%8x\n",
		index, arg,cval, mmc->resp[0], mmc->resp[1], mmc->resp[2], mmc->resp[3]);
#else
	udelay(5000);
#endif
	return 0;

}

static int __INIT__ s3c2440_mmc_init(void)
{
	__u32 val;

	val = readl(VA(S3C24XX_GPECON));
	val &= 0x3ffc00;
	val |= 0x2aa800;
	writel(VA(S3C24XX_GPECON), val);

	val = readl(VA(0x4c000000 + 0x0c));
	val |= 1 << 9;
	writel(VA(0x4c000000 + 0x0c), val);

	writel(VA(S3C24X0_SDIBASE + SDI_CON), 1 << 4 | 1);
	writeb(VA(S3C24X0_SDIBASE + SDI_PRE), 210);

	writel(VA(S3C24X0_SDIBASE + SDIDTIME), 0xff000);
	writel(VA(S3C24X0_SDIBASE + SDIBSIZE), 512);
	//writel(VA(S3C24X0_SDIBASE + SDIDATCON),2 << 22 | 7 << 18 | 3 << 16 | 1 << 14 | 3 << 12 | 0x100);
	val = readl(VA(S3C24X0_SDIBASE + SDIFSTA));
	val |= 1 << 16;
	writel(VA(S3C24X0_SDIBASE + SDIFSTA), val);

	udelay(1000);

	return mmc_register(&s3c2440_mmc);
}

module_init(s3c2440_mmc_init);
