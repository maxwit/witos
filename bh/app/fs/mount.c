#include <stdio.h>
#include <errno.h>
#include <fs/fs.h>

int main(int argc, char *argv[])
{
	int ret;
	const char *bdev_name;

	if (argc != 2)
	{
		printf("Usage:\n\t%s device\n", argv[0]);
		return -EINVAL;
	}

	bdev_name = argv[1];

	ret = mount("vfat", 0, bdev_name, NULL);
	if (ret < 0)
	{
		printf("fail to mount %s (ret = %d)\n", bdev_name, ret);
	}

	return ret;
}
