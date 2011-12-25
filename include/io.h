#pragma once

#include <types.h>

#if 0

#define VA(x)  ((__u32)(x))

static __u8 inline readb(__u32 addr)
{
	return *(volatile __u8 *)addr;
}

static void inline writeb(__u32 addr, __u8 val)
{
	*(volatile __u8 *)addr = val;
}

static __u16 inline readw(__u32 addr)
{
	return *(volatile __u16 *)addr;
}

static void inline writew(__u32 addr, __u16 val)
{
	*(volatile __u16 *)addr = val;
}

static __u32 inline readl(__u32 addr)
{
	return *(volatile __u32 *)addr;
}

static void inline writel(__u32 addr, __u32 val)
{
	*(volatile __u32 *)addr = val;
}

#else

#define VA(x)  ((void *)(x))

static __u8 inline readb(void *addr)
{
	return *(volatile __u8 *)addr;
}

static void inline writeb(void *addr, __u8 val)
{
	*(volatile __u8 *)addr = val;
}

static __u16 inline readw(void *addr)
{
	return *(volatile __u16 *)addr;
}

static void inline writew(void *addr, __u16 val)
{
	*(volatile __u16 *)addr = val;
}

static __u32 inline readl(void *addr)
{
	return *(volatile __u32 *)addr;
}

static void inline writel(void *addr, __u32 val)
{
	*(volatile __u32 *)addr = val;
}
#endif
