#pragma once

// #include <types.h>

int putchar(int);

char *gets(char *);

int puts(const char *);

int printf(const char *, ...);

int sprintf(char *, const char *, ...);

int snprintf(char *, int , const char *, ...);

int fflush(int);

void clear_screen(void);

#define BUG() \
	do \
	{ \
		printf(" BUG @ %s() line %d!\n", __func__, __LINE__); \
		while (1); \
	} while(0)

#ifdef CONFIG_DEBUG
#define DPRINT(fmt, args ...)	printf(fmt, ##args)

#define GEN_DGB() printf("%s(): line %d\n", __func__, __LINE__)

#define BUG_ON(x) \
	if (x) \
	{  \
		printf(" BUG @ %s() line %d!\n", __func__, __LINE__); \
		while(1); \
	}

#define assert(x) \
	if (!(x)) \
	{  \
		BUG(); \
	}
#else
#define DPRINT(fmt, args ...)
#define GEN_DGB()
#define BUG_ON(x)
#define assert(x)
#endif
