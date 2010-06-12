#pragma once

#include <types.h>

int putchar(int);

char *gets(char *);

int puts(const char *);

int printf(const char *, ...);

int sprintf(char *, const char *, ...);

int snprintf(char *, int , const char *, ...);

int fflush(int);

void ClearScreen(void);

#ifdef CONFIG_DEBUG
#define DPRINT(fmt, args ...)	printf(fmt, ##args)
#define GEN_DGB() printf("%s(): line %d\n", __FUNCTION__, __LINE__)
#else
#define DPRINT(fmt, args ...)
#define GEN_DGB()
#endif

