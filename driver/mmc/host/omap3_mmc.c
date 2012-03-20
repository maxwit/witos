#include <io.h>
#include <init.h>
#include <stdio.h>
#include <delay.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <mmc/mmc.h>

// #define CONFIG_MMC_DEBUG

#define CLK_INITSEQ             0
#define CLK_400KHZ              1
#define CLK_MISC                2

#define MMC_CLOCK_REFERENCE     (96)
#define MMC_INIT_SEQ_CLK        (MMC_CLOCK_REFERENCE * 1000 / 80)
#define MMC_400kHz_CLK          (MMC_CLOCK_REFERENCE * 1000 / 400)

#ifdef CONFIG_MMC_DEBUG
#define MMC_PRINT(fmt, args ...)	printf(fmt, ##args)
#else
#define MMC_PRINT(fmt, args ...)
#endif

#define omap3_mmc_read(offset)\
	readl(VA(MMCHS1_BASE + (offset)))

#define omap3_mmc_write(offset, val) \
	writel(VA(MMCHS1_BASE + (offset)), val)

static int omap3_send_cmd(struct mmc_host *mmc, __u32 index, __u32 arg, RESP resp);
static int omap3_mmc_read_data(struct mmc_host *mmc, void *buf);
static int omap3_mmc_write_data(struct mmc_host *mmc, const void *buf);
static void mmc_clock_config(__u32 iclk, __u16 clk_div);
static void omap3_set_hclk(void);
static void omap3_set_lclk(void);
static void omap3_soft_reset(void);

static struct mmc_host omap3_mmc = {
	.send_cmd = omap3_send_cmd,
	.read_data = omap3_mmc_read_data,
	.write_data = omap3_mmc_write_data,
	.set_hclk = omap3_set_hclk,
	.set_lclk = omap3_set_lclk,
};

static void omap3_set_hclk(void)
{
	mmc_clock_config(CLK_MISC, 20);

}

static void omap3_set_lclk(void)
{
	mmc_clock_config(CLK_400KHZ, 0);
}

static int omap3_mmc_write_data(struct mmc_host *mmc, const void *buf)
{
	__u32 val;
	int i, timeout = 0;

	do {
		val = omap3_mmc_read(MMCHS_STAT);
		MMC_PRINT("MMCHS_STAT = 0x%x %s(), line: %d\n", val, __FUNCTION__, __LINE__);
		udelay(1000);
		timeout++;
	} while (!(val & 1 << 4) && timeout < 5);

	if (val & (1 << 4)) {
		for (i = 0; i < 512 / 4; i++) {
			val = ((__u32*)buf)[i];
			omap3_mmc_write(MMCHS_DATA, val);
		}
	} else {
		MMC_PRINT("Write Buff is not ready!\n");
		return -EBUSY;
	}

	timeout = 0;
	while (1) {
		val = omap3_mmc_read(MMCHS_STAT);
		timeout++;
		udelay(1000);
		MMC_PRINT("MMCHS_STAT = 0x%x %s(), line: %d\n", val, __FUNCTION__, __LINE__);
		if ((val & (1 << 1)) || timeout > 5)
			break;
	}

	return 0;
}

static int omap3_mmc_read_data(struct mmc_host *mmc, void *buf)
{
	__u32 val;
	int i, timeout = 0;

	do {
		val = omap3_mmc_read(MMCHS_STAT);
		MMC_PRINT("MMCHS_STAT = 0x%x %s(), line: %d\n", val, __FUNCTION__, __LINE__);
		udelay(1000);
		timeout++;
	} while (!(val & 1 << 5) && timeout < 5);

	if (val & (1 << 5)) {
		for (i = 0; i < 512 / 4; i++) {
			val = omap3_mmc_read(MMCHS_DATA);
			((__u32*)buf)[i] = val;
		}
	}else
	{
		MMC_PRINT("Read Buff is not ready!\n");
		return -EBUSY;
	}

	timeout = 0;
	while (1) {
		val = omap3_mmc_read(MMCHS_STAT);
		timeout++;
		udelay(1000);
		MMC_PRINT("MMCHS_STAT = 0x%x %s(), line: %d\n", val, __FUNCTION__, __LINE__);
		if ((val & (1 << 1)) || timeout > 5)
			break;
	}
	val = omap3_mmc_read(MMCHS_STAT);
	omap3_mmc_write(MMCHS_STAT, val);

	return 0;
}

static int omap3_send_cmd(struct mmc_host *mmc, __u32 index, __u32 arg, RESP type)
{
	__u32 cval, val, timeout;

	cval = index << 24;

	switch (type) {
	case R1:
	case R4:
	case R5:
	case R6:
	case R7:
		cval |= 2 << 16 | 3 << 19;
		break;
	case R3:
		cval |= 2 << 16;
		break;
	case R2:
		cval |= 1 << 16 | 1 << 19;
		break;
	case R1b:
		cval |= 3 << 16 | 3 << 19;
		break;

	case REV:
	case NONE:
		break;
	}

	if (index == MMC_READ_SINGLE_BLOCK || index == MMC_READ_MULTIPLE_BLOCK)
		cval |= 1 << 21 | 1 << 4;

	if (index == MMC_WRITE_BLOCK || index == MMC_WRITE_MULTIPLE_BLOCK)
		cval |= 1 << 21;

	while (1) {
		timeout = 0;
		do {
			val = omap3_mmc_read(MMCHS_PSTATE);
			timeout++;
			MMC_PRINT("MMCHS_PSTATE = 0x%x %s(), line:%d\n", val,__FUNCTION__, __LINE__);
		} while ((val & (1 << 1)) && (timeout < 10));

		if (timeout == 10) {

			MMC_PRINT("timeout, reset data line!\n");
			val = omap3_mmc_read(MMCHS_SYSCTL);
			val |= 1 << 26;
			omap3_mmc_write(MMCHS_SYSCTL, val);

			do {
				val = omap3_mmc_read(MMCHS_SYSCTL);
				MMC_PRINT("MMCHS_SYSCTL = %x, line: %d\n", val, __LINE__);
			} while (val & (1 << 26));

			continue;
		}

		break;

	}
	omap3_mmc_write(MMCHS_STAT, 0xffffffff);
	omap3_mmc_write(MMCHS_BLK, 0x200);

	omap3_mmc_write(MMCHS_ARG, arg);
	omap3_mmc_write(MMCHS_CMD, cval);

	while (1) {
		val = omap3_mmc_read(MMCHS_STAT);
		if (val & (1 << 16)) {

			MMC_PRINT("Command %d send ERROR! reset command line!\n", index);

			if (1) {

				val = omap3_mmc_read(MMCHS_SYSCTL);
				val |= 1 << 25;
				omap3_mmc_write(MMCHS_SYSCTL, val);

				do {
					val = omap3_mmc_read(MMCHS_SYSCTL);
					MMC_PRINT("MMCHS_SYSCTL = %x, line: %d\n", val, __LINE__);
				} while (val & (1 << 25));

			}
			//omap3_soft_reset();
			return -EBUSY;

		} else if (val & 1)
		{

			omap3_mmc_write(MMCHS_STAT, 1);
			mmc->resp[0] = omap3_mmc_read(MMCHS_RSP10);
			mmc->resp[1] = omap3_mmc_read(MMCHS_RSP32);
			mmc->resp[2] = omap3_mmc_read(MMCHS_RSP54);
			mmc->resp[3] = omap3_mmc_read(MMCHS_RSP76);
			break;
		}
	}

#ifdef CONFIG_MMC_DEBUG
		MMC_PRINT("cmd%d: arg = 0x%x val =0x%x\nresp[0]=0x%8x\nresp[1]=0x%8x\nresp[2]=0x%8x\nresp[3]=0x%8x\n",
			index, arg, cval, mmc->resp[0], mmc->resp[1], mmc->resp[2], mmc->resp[3]);
#else
		udelay(8000);
#endif

	return 0;
}

static void mmc_clock_config(__u32 iclk, __u16 clk_div)
{
	__u32 val, cdiv;

	val = omap3_mmc_read(MMCHS_SYSCTL);
	val &= ~(1 << 2);
	omap3_mmc_write(MMCHS_SYSCTL, val);

	switch (iclk) {
	case CLK_INITSEQ:
		cdiv = MMC_INIT_SEQ_CLK / 2;
		break;
	case CLK_400KHZ:
		cdiv = MMC_400kHz_CLK;
		break;
	case CLK_MISC:
		cdiv = clk_div;
		break;
	default:
		return ;
	}

	val = omap3_mmc_read(MMCHS_SYSCTL);
	val &= ~(0x3ff << 6);
	val |= cdiv << 6;
	omap3_mmc_write(MMCHS_SYSCTL, val);

	do {
		val = omap3_mmc_read(MMCHS_SYSCTL);
		MMC_PRINT("MMCHS_SYSCTL = %x\n", val);
	} while (!(val & (1 << 1)));

	val = omap3_mmc_read(MMCHS_SYSCTL);
	val |= 1 << 2;
	omap3_mmc_write(MMCHS_SYSCTL, val);

}

static void omap3_soft_reset(void)
{
	__u32 val;
	val = omap3_mmc_read(MMCHS_SYSCTL);
	val |= 1 << 24;
	omap3_mmc_write(MMCHS_SYSCTL, val);

	do {
		val = omap3_mmc_read(MMCHS_SYSCTL);
		MMC_PRINT("MMCHS_SYSCTL = %x, line: %d\n", val, __LINE__);
	} while (val & (1 << 24));

}
static int omap3_mmc_init(void)
{
	__u32 val;
	int timeout = 0;
#if 0
	writew(VA(0x48002000 + 0x144), 3 << 3);

	for (val = 0x146; val <= 0x156; val += 2)
		writew(VA(0x48002000 + val), 1 << 8 | 3 << 3);
	val = readl(VA(0x48002520));
	val |= 1 << 9 | 3 << 1;
	writel(VA(0x48002520), val);

	val = readl(VA(0x48002274));
	val |= 1 << 24;
	writel(VA(0x48002274), val);
#else
	for (val = 0x144; val <= 0x156; val += 2)
		writew(VA(0x48002000 + val), 1 << 8);

#endif

	// Enable Functional and Interface clock for MMC Controller
	val = readl(VA(CM_FCLKEN1_CORE));
	val |= 1 << 24;
	writel(VA(CM_FCLKEN1_CORE), val);
	val = readl(VA(CM_ICLKEN1_CORE));
	val |= 1 << 24;
	writel(VA(CM_ICLKEN1_CORE), val);

	udelay(1000);

	val = omap3_mmc_read(MMCHS_SYSCONFIG);
	val |= 1;
	omap3_mmc_write(MMCHS_SYSCONFIG, val);

	do {
		val = omap3_mmc_read(MMCHS_SYSSTATUS);
		MMC_PRINT("MMCHS_SYSSTATUS = %x, %s(),line: %d\n", val, __FUNCTION__, __LINE__);
	} while (!(val & 1) );

	omap3_soft_reset();

	val = omap3_mmc_read(MMCHS_CAPA);
	val |= 3 << 25;
	omap3_mmc_write(MMCHS_CAPA, val);

	MMC_PRINT("MMCHS_CAPA = %x MMCHS_CUR_CAPA = %x\n",
			omap3_mmc_read(MMCHS_CAPA),
			omap3_mmc_read(MMCHS_CUR_CAPA));

	omap3_mmc_write(MMCHS_CON, 0x1);
	omap3_mmc_write(MMCHS_HCTL, 6 << 9 | 1 << 1);
	val = omap3_mmc_read(MMCHS_SYSCTL);
	val |= 1;
	omap3_mmc_write(MMCHS_SYSCTL, val);

	mmc_clock_config(CLK_INITSEQ, 0);

	val = omap3_mmc_read(MMCHS_HCTL);
	val |= 1 << 8;
	omap3_mmc_write(MMCHS_HCTL, val);

	timeout = 0;
	do {
		val = omap3_mmc_read(MMCHS_HCTL);
		timeout++;
		MMC_PRINT("MMCHS_HCTL = %x\n", val);
	} while (!(val & (1 << 8))&& timeout < 10);

	//omap3_mmc_write(MMCHS_IE, 0x317f0337);
	omap3_mmc_write(MMCHS_IE, 0x307f0033);

	//start initialization stream
	val = omap3_mmc_read(MMCHS_CON);
	val |= 1 << 1;
	omap3_mmc_write(MMCHS_CON, val);

	//send dummy command
	omap3_mmc_write(MMCHS_CMD, 0);
	do {
		val = omap3_mmc_read(MMCHS_STAT);
		MMC_PRINT("MMCHS_STAT = 0x%x %s(), line:%d\n", val, __FUNCTION__, __LINE__);
	} while (!(val & 1));

	val = omap3_mmc_read(MMCHS_STAT);
	val |= 1;
	omap3_mmc_write(MMCHS_STAT, val);

	omap3_mmc_write(MMCHS_CMD, 0);
	do {
		val = omap3_mmc_read(MMCHS_STAT);
		MMC_PRINT("MMCHS_STAT = 0x%x %s(), line:%d\n", val, __FUNCTION__, __LINE__);
	} while (!(val & 1));

	val = omap3_mmc_read(MMCHS_STAT);
	val |= 1;
	omap3_mmc_write(MMCHS_STAT, val);

	//end initialization stream
	val = omap3_mmc_read(MMCHS_CON);
	val &= ~(1 << 1);
	omap3_mmc_write(MMCHS_CON, val);

	//Configure clock to 400K = 96M /640

	mmc_register(&omap3_mmc);

	return 0;
}

module_init(omap3_mmc_init);
