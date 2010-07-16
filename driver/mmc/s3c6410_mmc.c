#include <g-bios.h>
#include <mmc/mmc.h>

//#define CONF_DEBUG
static int s3c6410_send_cmd(struct mmc_host *mmc, u32 index, u32 arg, RESP resp);
static void s3c6410_set_hclk(void);
static int s3c6410_mmc_read_data(struct mmc_host *mmc, void *buf);
static int s3c6410_mmc_write_data(struct mmc_host *mmc, void *buf);


static struct mmc_host s3c6410_mmc = {
	.send_cmd = s3c6410_send_cmd,
	.set_hclk = s3c6410_set_hclk,
	.read_data = s3c6410_mmc_read_data,
	.write_data = s3c6410_mmc_write_data,
};

static void s3c6410_set_hclk(void)
{
	u16 hval;

	writew(VA(MMC0_BASE + CLKCON), 0);

	writew(VA(MMC0_BASE + CLKCON), 0x8 << 8 | 1);
	do
	{
		hval = readw(VA(MMC0_BASE + CLKCON));
	} while (!(hval & 0x2));

	hval |= 1 << 2;

	writew(VA(MMC0_BASE + CLKCON), hval);

}

static int s3c6410_mmc_read_data(struct mmc_host *mmc, void *buf)
{
	u16 val;
	int i;

	do
	{
		val = readw(VA(MMC0_BASE + NORINTSTS));
		//printf("line %d :NORINTSTS = 0x%x\n", __LINE__, val);
		//printf("line %d :PRNSTS = 0x%x\n", __LINE__, readw(VA(MMC0_BASE + PRNSTS)));

	} while (!(val & (1 << 5)));

	writew(VA(MMC0_BASE + NORINTSTS), 1 << 5);

	for (i = 0; i < 512 / 4; i++)
	{
		((u32*)buf)[i] = readl(VA(MMC0_BASE + BDAT));
	}

	do
	{
		val = readw(VA(MMC0_BASE + NORINTSTS));
		//printf("line %d :NORINTSTS = 0x%x\n", __LINE__, val);

	} while (!(val & 0x2));

	writew(VA(MMC0_BASE + NORINTSTS), 0x2);

	return 0;
}

static int s3c6410_mmc_write_data(struct mmc_host *mmc, void *buf)
{
	u16 val;
	int i;

	do
	{
		val = readw(VA(MMC0_BASE + NORINTSTS));
		//printf("line %d :NORINTSTS = 0x%x\n", __LINE__, val);
		//printf("line %d :PRNSTS = 0x%x\n", __LINE__, readw(VA(MMC0_BASE + PRNSTS)));

	} while (!(val & (1 << 4)));

	writew(VA(MMC0_BASE + NORINTSTS), 1 << 4);

	for (i = 0; i < 512 / 4; i++)
	{
		writel(VA(MMC0_BASE + BDAT), ((u32*)buf)[i]);
	}

	do
	{
		val = readw(VA(MMC0_BASE + NORINTSTS));
		//printf("line %d :NORINTSTS = 0x%x\n", __LINE__, val);

	} while (!(val & 0x2));

	writew(VA(MMC0_BASE + NORINTSTS), 0x2);


	return 0;
}
static int s3c6410_send_cmd(struct mmc_host *mmc, u32 index, u32 arg, RESP resp)
{
	u16 cval, val;

	cval = index << 8 | 3 << 3;

	switch (resp)
	{
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

	do
	{
		val = readw(VA(MMC0_BASE + PRNSTS));
		//printf("PRNSTS = 0x%x\n", val);

	} while ((val & 0x1));


	if (MMC_CMD17 == index)
	{
		cval |= 1 << 5;

		writeb(VA(MMC0_BASE + TRANSMOD), 1 << 4);

		do
		{
			val = readw(VA(MMC0_BASE + PRNSTS));
			//printf("PRNSTS = 0x%x\n", val);
		} while ((val & 0x2));
	}

	if (MMC_CMD24 == index)
	{
		cval |= 1 << 5;

		writeb(VA(MMC0_BASE + TRANSMOD), 0);

		do
		{
			val = readw(VA(MMC0_BASE + PRNSTS));
			//printf("PRNSTS = 0x%x\n", val);

		} while ((val & 0x2));
	}

	writel(VA(MMC0_BASE + ARGUMENT), arg);
	writew(VA(MMC0_BASE + CMDREG), cval);

	do
	{
		val = readw(VA(MMC0_BASE + NORINTSTS));
		//printf("cmd %d line %d :NORINTSTS = 0x%x\n", index, __LINE__, val);

	} while (!(val & 0x1));

	writew(VA(MMC0_BASE + NORINTSTS), 1);

	udelay(100);

	if ((MMC_CMD2 == index) || (MMC_CMD9 == index) || (MMC_CMD10 == index))
	{
		mmc->resp[0] = readl(VA(MMC0_BASE + RESPREG0)) << 8;
		mmc->resp[1] = readl(VA(MMC0_BASE + RESPREG1)) << 8 | (readl(VA(MMC0_BASE + RESPREG0)) >> 24);
		mmc->resp[2] = readl(VA(MMC0_BASE + RESPREG2)) << 8 | (readl(VA(MMC0_BASE + RESPREG1)) >> 24);
		mmc->resp[3] = readl(VA(MMC0_BASE + RESPREG3)) << 8 | (readl(VA(MMC0_BASE + RESPREG2)) >> 24);
	}
	else
	{
		mmc->resp[0] = readl(VA(MMC0_BASE + RESPREG0));
		mmc->resp[1] = readl(VA(MMC0_BASE + RESPREG1));
		mmc->resp[2] = readl(VA(MMC0_BASE + RESPREG2));
		mmc->resp[3] = readl(VA(MMC0_BASE + RESPREG3));
	}
#ifdef CONF_DEBUG
		printf("cmd%d: arg = 0x%x val =0x%x\nresp[0]=0x%8x\nresp[1]=0x%8x\nresp[2]=0x%8x\nresp[3]=0x%8x\n",
			index, arg, cval, mmc->resp[0], mmc->resp[1], mmc->resp[2], mmc->resp[3]);
#else
		udelay(8000);
#endif

	return 0;
}
static int s3c6410_mmc_init(void)
{
	u16 hval;

	writew(VA(MMC0_BASE + CLKCON), 0x40 << 8 | 1);
	do
	{
		hval = readw(VA(MMC0_BASE + CLKCON));
	} while (!(hval & 0x2));

	hval |= 1 << 2;

	writew(VA(MMC0_BASE + CLKCON), hval);

	//uval = readl(VA(MMC0_BASE + CAPAREG));
	//printf("CAPAREG = 0x%x\n", uval);

	writeb(VA(MMC0_BASE + SWRST), 0x6);
	writeb(VA(MMC0_BASE + PWRCON), 0xf);
	writeb(VA(MMC0_BASE + HOSTCTRL), 0x2);
	//writel(VA(MMC0_BASE + CONTRL2), 1 << 31 | 3 << 14 | 1 << 12 | 1 << 8 | 1 << 3);
	//writel(VA(MMC0_BASE + CONTRL2), 3 << 14);
	//writel(VA(MMC0_BASE + CONTRL3), 1 << 7);

	writew(VA(MMC0_BASE + NORINTSIGEN), 0x33);
	writew(VA(MMC0_BASE + ERRINTSIGEN), 0x1ff);
	writew(VA(MMC0_BASE + NORINTSTSEN), 0x33);
	writew(VA(MMC0_BASE + ERRINTSTSE), 0x1);

	writew(VA(MMC0_BASE + BLKSIZE), 512);
	writew(VA(MMC0_BASE + BLKCNT), 1);

	mmc_register(&s3c6410_mmc);

	return 0;
}

DRIVER_INIT(s3c6410_mmc_init);

