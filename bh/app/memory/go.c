#include <string.h>

static void show_usage(void)
{
	printf("Usage:go [address]\n");
#if 0
	if address is not specified, jump to the default jump address!
#endif
}

int main(int argc, char *argv[])
{
	int ret;
	void (*fn)(void);

	if (argc != 2)
	{
		show_usage();

		return -EINVAL;
	}

	ret = string2value(argv[1], (u32 *)&fn);
	if (ret < 0)
	{
		printf("mem_addr is invalid!\n");
		show_usage();

		return -EINVAL;
	}
	printf("goto 0x%08x ...\n", (u32)fn);

	fn();

	return -EIO;
}
