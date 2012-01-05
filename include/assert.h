#pragma once

#include <stdio.h>

#define BUG() \
	do { \
		printf(" BUG @ %s() line %d!\n", __func__, __LINE__); \
		while (1); \
	} while(0)

#define assert(x) \
	do { if (!(x)) BUG(); } while (0)
