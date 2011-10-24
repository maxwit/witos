#include <string.h>

int main(int argc, char *argv[])
{
	int ret;
	void (*fn)(void);

	if (argc != 2)
	{
		usage();

		return -EINVAL;
	}

	ret = str_to_val(argv[1], (__u32 *)&fn);
	if (ret < 0)
	{
		printf("mem_addr is invalid!\n");
		usage();

		return -EINVAL;
	}
	printf("goto 0x%08x ...\n", (__u32)fn);

	fn();

	return -EIO;
}
