#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ext2.h"

#define BUF_LEN 512
#define MNTPT   "c"

int block_device_init();
int disk_drive_init();
int foo_drv_init(const char *);
int ext2_fs_init(void);
int fat_fs_init(void);

int main(int argc, char *argv[])
{
	int ret, fd;
	ssize_t len;
	const char *disk, *type;
	char buff[BUF_LEN], part[BUF_LEN], path[BUF_LEN];

	if (argc != 5)
	{
		printf("usage:\n\t%s disk partno type file\n", argv[0]);
		return -EINVAL;
	}

	disk = argv[1];
	snprintf(part, BUF_LEN, "foo0p%s", argv[2]);
	type = argv[3];
	snprintf(path, BUF_LEN, MNTPT ":%s", argv[4]);

	block_device_init();
	disk_drive_init();
	ret = foo_drv_init(disk);
	if (ret < 0)
	{
		return ret;
	}

	ret = ext2_fs_init();
	if (ret < 0)
	{
		return ret;
	}

	ret = fat_fs_init();
	if (ret < 0)
	{
		return ret;
	}

	ret = demo_mount(type, 0, part, MNTPT);
	if (ret < 0)
	{
		printf("fail to mount %s with %s! (ret = %d)\n", part, type, ret);
		return ret;
	}

	fd = demo_open(path, 0);
	if (fd < 0)
	{
		printf("fail to open %s\n", path);
		return fd;
	}

	len = demo_read(fd, buff, BUF_LEN);

	if (len > 0)
	{
		buff[len] = '\0';
		printf("data:\n\t%s\n", buff);
	}
	printf("%s() line %d\n", __func__, __LINE__);

	demo_close(fd);

	demo_umount(MNTPT);

	return 0;
}
