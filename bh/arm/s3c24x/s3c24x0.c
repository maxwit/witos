#include <arm/s3c24x0.h>

static void s3c24x_reboot(void)
{
	__u32 val;

	val = readl(VA(WATCHDOG_BASE + WTCON));
	val = val | (1 << 5) | 1;
	writel(VA(WATCHDOG_BASE + WTCON), val);

	while(1);
}

DECLARE_REBOOT(s3c24x_reboot);
