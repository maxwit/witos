#pragma once

#include <types.h>

#if 0
// tmp
static inline void udelay(__u32 n)
{
	volatile long m = n;

	while (m-- >= 0);
}
#else
void udelay(__u32 n);
#endif

void mdelay(__u32 n);

#define ndelay udelay

__u32 get_tick(void);

void inc_tick(void);

void calibrate_delay(__u32);
