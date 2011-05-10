#include <timer.h>

int main(int argc, char *argv[])
{
#ifdef CONFIG_IRQ_SUPPORT
	volatile u32 count;

#if 1
	for (count = 0; count < 10000; count++)
	{
		printf("%s(): %d\n", __func__, get_tick());
	}
#else
	volatile u32 m = get_tick() + 10;

	do
	{
		count = get_tick();
		if (count & 1)
			printf("%d, %d\n", count, m);
	} while (count < m);
#endif
#endif
	return 0;
}
