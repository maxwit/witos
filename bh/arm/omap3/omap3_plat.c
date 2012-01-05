#include <io.h>
#include <irq.h>
#include <init.h>
#include <platform.h>
#include <board.h>

static int __INIT__ omap3_init(void)
{
	int ret;
	__u32 val;

	val = readl(VA(PADCONF_GPMC_NBE1));
	val |= 0xffff0000;
	writel(VA(PADCONF_GPMC_NBE1), val);

	val = readl(VA(PADCONF_GPMC_NBE1));

	// GPMC
	for (val = 0x7A; val <= 0x8C; val += 2)
		writew(VA(0x48002000 + val), 0);

	for (; val <= 0xAC; val += 2)
		writew(VA(0x48002000 + val), 1 << 8);

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

	ret = board_init();

	return ret;
}

PLAT_INIT(omap3_init);

static void omap3_reset(void)
{
	__u32 val;

	val = readl(VA(PRM_BASE + PRM_RSTCTRL));
	val |= 0x1 << 1;
	writel(VA(PRM_BASE + PRM_RSTCTRL), val);
}

DECL_RESET(omap3_reset);
