#include <mmc/mmc.h>

#define AT91_MCI_BASE 0xFFFA8000
#define AT91_MCI_CR    0x00
#define AT91_MCI_MR    0x04
#define AT91_MCI_DTOR  0x08
#define AT91_MCI_SDCR  0x0C
#define AT91_MCI_ARGR  0x10
#define AT91_MCI_CMDR  0x14
#define AT91_MCI_RSPR(n)  (0x20 + 4 * n)
#define AT91_MCI_SR    0x40
#define AT91_MCI_IER   0x44
#define AT91_MCI_IDR   0x48
#define AT91_MCI_IMR   0x4C

#define MAXLAT    (0 << 12)
#define RSPTYP48  (1 << 6)
#define RSPTYP136 (2 << 6)

#define at91_mci_read(reg) \
		readl(VA(AT91_MCI_BASE + reg))

#define at91_mci_write(reg, val) \
		writel(VA(AT91_MCI_BASE + reg), val)

static int at91_mci_cmd(struct mmc_host *mmc, __u32 cmd, __u32 arg)
{
	__u32 cmdr = cmd | MAXLAT, stat;

	switch (cmd) {
	case MMC_CMD2:
		cmdr |= RSPTYP136;
		break;

	case MMC_CMD3:
	case MMC_CMD8:
	case MMC_CMD41:
	case MMC_CMD55:
		cmdr |= RSPTYP48;
		break;

	default:
		break;
	}

	at91_mci_write(AT91_MCI_ARGR, arg);
	at91_mci_write(AT91_MCI_CMDR, cmdr);

	while (1) {
		stat = at91_mci_read(AT91_MCI_SR);
		if (stat & 0x1)
			break;

		udelay(100);
	}

	mmc->resp[0] = at91_mci_read(AT91_MCI_RSPR(0));
	mmc->resp[1] = at91_mci_read(AT91_MCI_RSPR(1));
	mmc->resp[2] = at91_mci_read(AT91_MCI_RSPR(2));
	mmc->resp[3] = at91_mci_read(AT91_MCI_RSPR(3));

#define CONFIG_DEBUG

#ifdef CONFIG_DEBUG
	printf("\nCMD%d:\nCMDr = 0x%08x\nARGr = 0x%08x\nSTAr = 0x%08x\n"
		"RSPr = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
		cmd, cmdr, arg, stat,
		mmc->resp[0], mmc->resp[1], mmc->resp[2], mmc->resp[3]);
#endif

	return 0;
}

static int at91_mci_isr(__u32 irq, void *at91_mci)
{
	__u32 stat = at91_mci_read(AT91_MCI_SR);
	__u32 mask = at91_mci_read(AT91_MCI_IMR);

	printf("%s(): status = 0x%08x, mask = 0x%08x\n",
		__func__, stat, mask);

	return 0;
}

static struct mmc_host at91_mci =
{
	.send_cmd = at91_mci_cmd,
};

static int __INIT__ at91_mci_init(void)
{
	__u32 val;

	at91_gpio_conf_periB(PIOA, 0x77, 0);

#ifdef CONFIG_AT91SAM9261
	at91_clock_enable(PID_MCI);
#elif defined(CONFIG_AT91SAM9263)
#warning
	at91_clock_enable(PID_MCI1);
#endif

	at91_mci_write(AT91_MCI_CR, 1);
	val = at91_mci_read(AT91_MCI_MR);
	val = val & ~0xff | 124;
	at91_mci_write(AT91_MCI_MR, val);
	at91_mci_write(AT91_MCI_SDCR, 0);

	return mmc_register(&at91_mci);
}

module_init(at91_mci_init);

