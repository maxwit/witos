#include <random.h>

#define RAND_MAX 32767

static unsigned long g_seed = 1;

int random(void)
{
	g_seed = g_seed * 1103515245 + 12345;
	return (unsigned)(g_seed / ((RAND_MAX + 1) * 2)) % (RAND_MAX + 1);
}

void srandom(unsigned seed)
{
	g_seed = seed;
}
