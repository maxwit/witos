#include <string.h>

static void usage(int argc, char *argv[])
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
		usage(argc, argv);

		return -EINVAL;
	}

	ret = str_to_val(argv[1], (u32 *)&fn);
	if (ret < 0)
	{
		printf("mem_addr is invalid!\n");
		usage(argc, argv);

		return -EINVAL;
	}
	printf("goto 0x%08x ...\n", (u32)fn);

	fn();

	return -EIO;
}
