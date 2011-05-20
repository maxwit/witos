#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ext2.h"
#include "block.h"

int main(int argc, char *argv[])
{
	char buff[DISK_BLOCK_SIZE];
	struct ext2_dir_entry *dir;
	struct block_device *bdev;

	if (argc != 2)
	{
		printf("usage ..\n");
		return -EINVAL;
	}

	bdev = bdev_open(argv[1]);
	if (bdev == NULL)
	{
		char str[64];

		sprintf(str, "open(%s)", argv[1]);
		perror(str);

		return -ENODEV;
	}

	dir = ext2_mount(bdev, "ext2");

	bdev_close(bdev);

	return 0;
}
