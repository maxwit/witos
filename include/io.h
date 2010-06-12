#pragma once

#include <types.h>

#define WAIT_INFINITE   0
#define WAIT_ASYNC     -1

#if CONFIG_GTH

#define VA(x)  ((u32)(x))

static u8 __INLINE__ readb(u32 addr)
{
	return *(volatile u8 *)addr;
}

static void __INLINE__ writeb(u32 addr, u8 val)
{
	*(volatile u8 *)addr = val;
}

static u16 __INLINE__ readw(u32 addr)
{
	return *(volatile u16 *)addr;
}

static void __INLINE__ writew(u32 addr, u16 val)
{
	*(volatile u16 *)addr = val;
}

static u32 __INLINE__ readl(u32 addr)
{
	return *(volatile u32 *)addr;
}

static void __INLINE__ writel(u32 addr, u32 val)
{
	*(volatile u32 *)addr = val;
}

#else

#define VA(x)  ((void *)(x))

static u8 __INLINE__ readb(void *addr)
{
	return *(volatile u8 *)addr;
}

static void __INLINE__ writeb(void *addr, u8 val)
{
	*(volatile u8 *)addr = val;
}

static u16 __INLINE__ readw(void *addr)
{
	return *(volatile u16 *)addr;
}

static void __INLINE__ writew(void *addr, u16 val)
{
	*(volatile u16 *)addr = val;
}

static u32 __INLINE__ readl(void *addr)
{
	return *(volatile u32 *)addr;
}

static void __INLINE__ writel(void *addr, u32 val)
{
	*(volatile u32 *)addr = val;
}

#endif

