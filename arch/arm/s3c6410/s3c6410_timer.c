#include <g-bios.h>

void udelay(u32 n)
{
	volatile u32 m = n * 0x1000;

	while (m-- > 0);
}


int __INIT__ s3c6410_timer_init(void)
{
	return 0;
}
