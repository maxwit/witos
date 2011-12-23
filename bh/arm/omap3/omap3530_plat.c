#include <irq.h>
#include <init.h>
#include <io.h>
#include <flash/flash.h>
#include <platform.h>

#ifdef CONFIG_BOARD_EVM3530
static struct platform_device lan9220_device = {
	.dev = {
		.memio = 0x28000000,
		.irq = GPIO_IRQ(19),
	},
	.name = "SMSC LAN9220",
};

static int __INIT__ evm3530_device_init(void)
{
	int ret;

	ret = platform_device_register(&lan9220_device);
	return ret;
}

POSTSUBS_INIT(evm3530_device_init);
#endif

static int __INIT__ omap3530_init(void)
{
	__u32 val, val4;
#ifdef CONFIG_LAN9220
	__u16 val2;
#endif

	//
	val4 = readl(VA(PADCONF_GPMC_NBE1));
	DPRINT("GPIO62 = 0x%08x\n", val4);
	val4 |= 0xffff0000;
	writel(VA(PADCONF_GPMC_NBE1), val4);

	val4 = readl(VA(PADCONF_GPMC_NBE1));
	DPRINT("GPIO62 = 0x%08x\n", val4);

	// GPMC
	for (val4 = 0x7A; val4 <= 0x8C; val4 += 2)
		writew(VA(0x48002000 + val4), 0);

	for (; val4 <= 0xAC; val4 += 2)
		writew(VA(0x48002000 + val4), 1 << 8);

#if defined(CONFIG_BOARD_BEAGLE) && defined(CONFIG_SMSC91X)
	val4 = readl(VA(GPMC_CONFIG1(1)));
	val4 &= ~(0x3 << 10);
	val4 &= ~(0x1 << 9);
	val4 |= 0x1;
	writel(VA(GPMC_CONFIG1(1)), val4);

	val4 = 0x8 << 8 | 0x1 << 6 | 0x8;
	writel(VA(GPMC_CONFIG7(1)), val4);
#elif defined(CONFIG_BOARD_DEVKIT8000) && defined(CONFIG_DM9000)
	writew(VA(0x480020BA), 1 << 4 | 1 << 3 | 0);

	writel(VA(0x6E000060 + 6 * 0x30 + 0x0), 0x00001000);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x04), 0x001e1e00);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x08), 0x00080300);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x0c), 0x1c091c09);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x10), 0x04181f1f);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x14), 0x00000FcF);
	writel(VA(0x6E000060 + 6 * 0x30 + 0x18), 0x00000f6c);
#elif defined(CONFIG_BOARD_EVM3530) && defined(CONFIG_LAN9220)
	writew(VA(0x480020B8), 1 << 4 | 1 << 3 | 0);

	writel(VA(0x6E000060 + 5 * 0x30 + 0x00), 0x00001000);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x04), 0x001e1e01);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x08), 0x00080300);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x0c), 0x1c091c09);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x10), 0x04181f1f);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x14), 0x00000fcf);
	writel(VA(0x6E000060 + 5 * 0x30 + 0x18), 0x00000f68);

	val2 = readw(VA(PADCONF_GPMC_NWE));
	val2 |= 0x0E00;
	writew(VA(PADCONF_GPMC_NWE), val2);

	val2 = readw(VA(PADCONF_GPMC_NOE));
	val2 |= 0x0E00;
	writew(VA(PADCONF_GPMC_NOE), val2);

	val2 = readw(VA(PADCONF_GPMC_NADV_ALE));
	val2 |= 0x0E00;
	writew(VA(PADCONF_GPMC_NADV_ALE), val2);
#endif

	// enable gpio1's clock(function,interface and auto wakeup clock)
	val = readl(VA(0x48004c00));
	val |= 1 << 3;
	writel(VA(0x48004c00), val);

	val = readl(VA(0x48004c10));
	val |= 1 << 3;
	writel(VA(0x48004c10), val);

	val = readl(VA(0x48004c30));
	val |= 1 << 3;
	writel(VA(0x48004c30), val);

#ifdef CONFIG_IRQ_SUPPORT
	omap3_irq_init();
	irq_enable();
#endif

	return 0;
}

PLAT_INIT(omap3530_init);

static void omap3530_reset(void)
{
	__u32 val;

	val = readl(VA(PRM_BASE + PRM_RSTCTRL));
	val |= 0x1 << 1;
	writel(VA(PRM_BASE + PRM_RSTCTRL), val);
}

DECL_RESET(omap3530_reset);
