#include <arm/s3c24x0.h>

static void s3c24x_reset(void)
{
	__u32 val;

	val = readl(VA(WATCHDOG_BASE + WTCON));
	val = val | (1 << 5) | 1;
	writel(VA(WATCHDOG_BASE + WTCON), val);

	while(1);
}

DECL_RESET(s3c24x_reset);
