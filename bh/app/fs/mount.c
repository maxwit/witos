#include <stdio.h>
#include <errno.h>
#include <fs/fs.h>

int main(int argc, char *argv[])
{
	int ret;
	struct block_device *bdev;

	if (argc != 2)
	{
		printf("Usage:\n\t%s device\n", argv[1]);
		return -EINVAL;
	}

	bdev = get_bdev_by_name(argv[1]);
	if (NULL == bdev)
	{
		printf("fail to open block device \"%s\"!\n", argv[1]);
		return -ENODEV;
	}

	ret = fat_mount(bdev, "vfat", 0);


	return ret;
}
