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

static int fs_init(void)
{
	int ext2_fs_init(void);
	int fat_fs_init(void);

	return ext2_fs_init() && fat_fs_init();
}

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

	fs_init();

	ret = demo_mount(type, 0, part, MNTPT);
	if (ret < 0)
	{
		printf("fail to mount foo0p2! (ret = %d)\n", ret);
		return ret;
	}

	// snprintf(path, BUF_LEN, "foo0p2:%s", path);
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
