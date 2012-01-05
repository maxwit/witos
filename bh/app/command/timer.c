#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <timer.h>

int main(int argc, char *argv[])
{
#ifdef CONFIG_TIMER_SUPPORT
	volatile __u32 count;

#if 1
	for (count = 0; count < 10000; count++)
		printf("%s(): %d\n", __func__, get_tick());
#else
	volatile __u32 m = get_tick() + 10;

	do {
		count = get_tick();
		if (count & 1)
			printf("%d, %d\n", count, m);
	} while (count < m);
#endif
#else
	printf("Timer not support on this platform!\n");
#endif

	return 0;
}
