#pragma once

#include <stdio.h>

#define BUG() \
	do { \
		printf("Bug @ %s() line %d of %s!\n", __func__, __LINE__, __FILE__); \
		while (1); \
	} while(0)

#define assert(x) \
	do { if (!(x)) BUG(); } while (0)
