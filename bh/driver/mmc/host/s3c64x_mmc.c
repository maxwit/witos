#include <mmc/mmc.h>
//#define CONFIG_MMC_DEBUG
#ifdef CONFIG_MMC_DEBUG
#define MMC_PRINT(fmt, args ...)	printf(fmt, ##args)
#else
#define MMC_PRINT(fmt, args ...)
#endif

static int s3c6410_send_cmd(struct mmc_host *mmc, __u32 index, __u32 arg, RESP resp);
static void s3c6410_set_hclk(void);
static void s3c6410_set_lclk(void);
static int s3c6410_mmc_read_data(struct mmc_host *mmc, void *buf);
static int s3c6410_mmc_write_data(struct mmc_host *mmc, const void *buf);

static struct mmc_host s3c6410_mmc = {
	.send_cmd = s3c6410_send_cmd,
	.set_hclk = s3c6410_set_hclk,
	.set_lclk = s3c6410_set_lclk,
	.read_data = s3c6410_mmc_read_data,
	.write_data = s3c6410_mmc_write_data,
};

static void s3c6410_config_clk(__u8 clk_div)
{
	__u16 hval;

	writew(VA(MMC0_BASE + CLKCON), (clk_div << 8) | 1);
	do {
		hval = readw(VA(MMC0_BASE + CLKCON));
	} while (!(hval & 0x2));

	hval |= 1 << 2;

	writew(VA(MMC0_BASE + CLKCON), hval);

}

static void s3c6410_set_lclk(void)
{
	s3c6410_config_clk(0x40);

}
static void s3c6410_set_hclk(void)
{
	s3c6410_config_clk(0x8);

}

static int s3c6410_mmc_read_data(struct mmc_host *mmc, void *buf)
{
	__u16 val;
	int i;

	do {
		val = readw(VA(MMC0_BASE + NORINTSTS));
		udelay(1000);
		MMC_PRINT("line %d :NORINTSTS = 0x%x\n", __LINE__, val);
		//MMC_PRINT("line %d :PRNSTS = 0x%x\n", __LINE__, readw(VA(MMC0_BASE + PRNSTS)));

		if (val & (1 << 15)) {

			writeb(VA(MMC0_BASE + SWRST), 1 << 2);
			MMC_PRINT("Error, Reset Data line!\n");
			return -EBUSY;
		}
	} while (!(val & (1 << 5)));

	writew(VA(MMC0_BASE + NORINTSTS), 1 << 5);

	for (i = 0; i < 512 / 4 ; i++)
		((__u32*)buf)[i] = readl(VA(MMC0_BASE + BDAT));

	do {
		val = readw(VA(MMC0_BASE + NORINTSTS));
		MMC_PRINT("line %d :NORINTSTS = 0x%x\n", __LINE__, val);
		if (val & (1 << 15)) {

			writeb(VA(MMC0_BASE + SWRST), 1 << 2);
			MMC_PRINT("Error, Reset Data line!\n");
			return -EBUSY;
		}

	} while (!(val & 0x2));

	writew(VA(MMC0_BASE + NORINTSTS), 0x2);

	return 0;
}

static int s3c6410_mmc_write_data(struct mmc_host *mmc, const void *buf)
{
	__u16 val;
	int i;

	do {
		val = readw(VA(MMC0_BASE + NORINTSTS));
		MMC_PRINT("line %d :NORINTSTS = 0x%x\n", __LINE__, val);
		//MMC_PRINT("line %d :PRNSTS = 0x%x\n", __LINE__, readw(VA(MMC0_BASE + PRNSTS)));
		if (val & (1 << 15)) {

			writeb(VA(MMC0_BASE + SWRST), 1 << 2);
			MMC_PRINT("Error, Reset Data line!\n");
			return -EBUSY;
		}

	} while (!(val & (1 << 4)));

	writew(VA(MMC0_BASE + NORINTSTS), 1 << 4);

	for (i = 0; i < 512 / 4; i++)
		writel(VA(MMC0_BASE + BDAT), ((__u32*)buf)[i]);

	do {
		val = readw(VA(MMC0_BASE + NORINTSTS));
		MMC_PRINT("line %d :NORINTSTS = 0x%x\n", __LINE__, val);

		if (val & (1 << 15)) {

			writeb(VA(MMC0_BASE + SWRST), 1 << 2);
			//MMC_PRINT("Error, Reset Data line!\n");
			//return -EBUSY;
			break;
		}

	} while (!(val & 0x2));

	writew(VA(MMC0_BASE + NORINTSTS), 0x2);

	return 0;
}
static int s3c6410_send_cmd(struct mmc_host *mmc, __u32 index, __u32 arg, RESP resp)
{
	__u16 cval, val;

	cval = index << 8 | 3 << 3;

	switch (resp) {
	case R1:
	case R3:
	case R4:
	case R5:
	case R6:
	case R7:
		cval |= 2;
		break;
	case R2:
		cval |= 1;
		break;
	case R1b:
		cval |= 3;
		break;

	case REV:
	case NONE:
		break;
	}

	do {
		val = readw(VA(MMC0_BASE + PRNSTS));
		MMC_PRINT("line %d:PRNSTS = 0x%x\n", __LINE__, val);

	} while ((val & 0x1));

	if ((MMC_READ_SINGLE_BLOCK== index) || (MMC_WRITE_BLOCK == index)) {
		cval |= 1 << 5;

		writew(VA(MMC0_BASE + BLKSIZE), 0x200);

		if (MMC_READ_SINGLE_BLOCK== index) {
			writew(VA(MMC0_BASE + TRANSMOD), 1 << 4);

		} else
		{
			writeb(VA(MMC0_BASE + TRANSMOD), 0);
		}
#if 1
{
		int timeout = 0;

		do {
			val = readw(VA(MMC0_BASE + PRNSTS));
			timeout++;
			udelay(100);
			MMC_PRINT("line %d:PRNSTS = 0x%x\n", __LINE__, val);
		} while ((val & 0x2) && timeout < 5);

		if (timeout == 5) {
			writeb(VA(MMC0_BASE + SWRST), 1 << 2);
			MMC_PRINT("Command %d Timeout, Reset Data line!\n", index);
			return -EBUSY;
		}
}
#endif
	}

	writel(VA(MMC0_BASE + ARGUMENT), arg);
	writew(VA(MMC0_BASE + CMDREG), cval);

	do {
		val = readw(VA(MMC0_BASE + NORINTSTS));
		MMC_PRINT("cmd %d line %d :NORINTSTS = 0x%x\n", index, __LINE__, val);

	} while (!(val & 0x1));

	writew(VA(MMC0_BASE + NORINTSTS), 1);

	udelay(100);

	mmc->resp[0] = readl(VA(MMC0_BASE + RESPREG0));
	mmc->resp[1] = readl(VA(MMC0_BASE + RESPREG1));
	mmc->resp[2] = readl(VA(MMC0_BASE + RESPREG2));
	mmc->resp[3] = readl(VA(MMC0_BASE + RESPREG3));

#ifdef CONFIG_MMC_DEBUG
		MMC_PRINT("cmd%d: arg = 0x%x val =0x%x\nresp[0]=0x%8x\nresp[1]=0x%8x\nresp[2]=0x%8x\nresp[3]=0x%8x\n",
			index, arg, cval, mmc->resp[0], mmc->resp[1], mmc->resp[2], mmc->resp[3]);
#else
		udelay(8000);
#endif
	if ((MMC_ALL_SEND_CID== index) || (MMC_SEND_CID== index) || (MMC_SEND_CSD == index) || (MMC_READ_DAT_UNTIL_STOP == index)) {

		mmc->resp[3] = ((mmc->resp[3] << 8)&(~0xff)) | ((mmc->resp[2] >> 24)&0xff);
		mmc->resp[2] = ((mmc->resp[2] << 8)&(~0xff)) | ((mmc->resp[1] >> 24)&0xff);
		mmc->resp[1] = ((mmc->resp[1] << 8)&(~0xff)) | ((mmc->resp[0] >> 24)&0xff);
		mmc->resp[0] = (mmc->resp[0] << 8)&(~0xff);
#ifdef CONFIG_MMC_DEBUG
		MMC_PRINT("cmd%d: arg = 0x%x val =0x%x\nresp[0]=0x%8x\nresp[1]=0x%8x\nresp[2]=0x%8x\nresp[3]=0x%8x\n",
			index, arg, cval, mmc->resp[0], mmc->resp[1], mmc->resp[2], mmc->resp[3]);
#endif
	}

	return 0;
}
#ifdef CONFIG_GTH
int mmc_init(void)
#else
static int s3c6410_mmc_init(void)
#endif
{
	//uval = readl(VA(MMC0_BASE + CAPAREG));
	//MMC_PRINT("CAPAREG = 0x%x\n", uval);
	s3c6410_set_lclk();

	writeb(VA(MMC0_BASE + SWRST), 0x6);
	writeb(VA(MMC0_BASE + PWRCON), 0xf);
	writeb(VA(MMC0_BASE + HOSTCTRL), 0x0);
	//writel(VA(MMC0_BASE + CONTRL2), 1 << 31 | 3 << 14 | 1 << 12 | 1 << 8 | 1 << 3);
	//writel(VA(MMC0_BASE + CONTRL2), 3 << 14);
	//writel(VA(MMC0_BASE + CONTRL3), 1 << 7);

	writew(VA(MMC0_BASE + NORINTSIGEN), 0x33);
	writew(VA(MMC0_BASE + ERRINTSIGEN), 0x1ff);
	writew(VA(MMC0_BASE + NORINTSTSEN), 0x33);
	writew(VA(MMC0_BASE + ERRINTSTSE), 0x11);

	mmc_register(&s3c6410_mmc);

	return 0;
}

#ifndef CONFIG_GTH
module_init(s3c6410_mmc_init);
#endif

