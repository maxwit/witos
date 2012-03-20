#include <types.h>

void __WEAK__ udelay(__u32 n)
{
	volatile __u32 m = n * (HCLK_RATE >> 20) >> 6;

	while (m-- > 0);
}

void __WEAK__ mdelay(__u32 n)
{
	udelay(1000 * n);
}
