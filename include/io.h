#pragma once

#include <types.h>

static inline __u8 readb(void *mem)
{
	return *(volatile __u8 *)mem;
}

static void inline writeb(void *mem, __u8 val)
{
	*(volatile __u8 *)mem = val;
}

static inline __u16 readw(void *mem)
{
	return *(volatile __u16 *)mem;
}

static void inline writew(void *mem, __u16 val)
{
	*(volatile __u16 *)mem = val;
}

static inline __u32 readl(void *mem)
{
	return *(volatile __u32 *)mem;
}

static void inline writel(void *mem, __u32 val)
{
	*(volatile __u32 *)mem = val;
}
