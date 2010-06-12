#pragma once

#ifdef __arm__
#include <arm/types.h>
#else
#error "arch not supported yet"
#endif

#define MKFOURCC(a, b, c, d)    (((a) << 24) | (b) << 16 | ((c) << 8) | (d))

#define __INLINE__        inline
#define __PACKED__        __attribute__((packed))
#define __WEAK__          __attribute__((weak))

// #define __FUNCTION__  __func__
// #define PRINT_ARG_FORMAT __attribute__((format (printf, 1, 2)))

#define DECLARE_REBOOT(func) \
	void reboot(void) __attribute__((alias(#func)))

#define NULL  ((void *)0)

#define ARRAY_ELEM_NUM(arr) (sizeof(arr) / sizeof((arr)[0]))

#define OFF2BASE(ptr, type, member) \
	(type *)((char *)ptr - (char *)(&((type *)0)->member))


#define min_t(type, x, y) \
		({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })

#define max_t(type, x, y) \
		({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#define KB(n) ((n) << 10)
#define MB(n) ((n) << 20)
#define GB(n) ((n) << 30)


#define BUG() \
	do \
	{ \
		printf(" BUG @ %s() line %d!\n", __FUNCTION__, __LINE__); \
		while (1); \
	} while(0)


#ifdef CONFIG_DEBUG
#define BUG_ON(x) \
	if (x) \
	{  \
		printf(" BUG @ %s() line %d!\n", __FUNCTION__, __LINE__); \
		while(1); \
	}
#else
#define BUG_ON(x)
#endif
