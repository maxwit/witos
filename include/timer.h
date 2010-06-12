#pragma once

#include <types.h>

#if 0
// tmp
static inline void udelay(u32 n)
{
	volatile long m = n;

	while (m-- >= 0);
}
#else
void udelay(u32 n);
#endif

void mdelay(u32 n);

#define ndelay udelay

u32 get_tick(void);

void inc_tick(void);

