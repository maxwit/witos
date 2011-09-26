#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "ext2.h"

#define BUF_LEN 512

int block_device_init();
int disk_drive_init();
int foo_drv_init(const char *);

int ext2_fs_init(void);

int mount(const char *type, unsigned long flags, const char *bdev_name, const char *path);

int main(int argc, char *argv[])
{
	int ret;
	struct ext2_file *fp;
	const char *dev, *fn;
	char path[BUF_LEN];
	ssize_t len;
	char buff[BUF_LEN];

	if (argc != 3)
	{
		printf("usage ..\n");
		return -EINVAL;
	}

	dev = argv[1];
	fn  = argv[2];

	block_device_init();
	disk_drive_init();
	foo_drv_init(dev);

	ext2_fs_init();
	printf("%s() line %d\n", __func__, __LINE__);

	ret = mount("ext2", 0, "foo0p2", 0);
	if (ret < 0)
	{
		printf("fail to mount foo0p2! (ret = %d)\n", ret);
		return ret;
	}
	printf("%s() line %d\n", __func__, __LINE__);

	// snprintf(path, BUF_LEN, "foo0p2:%s", fn);
	fp = ext2_open(fn, 0);
	if (NULL == fp)
	{
		printf("fail to open %s\n", path);
		return -ENOENT;
	}
	printf("%s() line %d\n", __func__, __LINE__);

	len = ext2_read(fp, buff, BUF_LEN);
	printf("data:\n\t%s\n", buff);

	if (len > 0)
	{
		buff[len] = '\0';
		printf("data:\n\t%s\n", buff);
	}
	printf("%s() line %d\n", __func__, __LINE__);

	ext2_close(fp);

	// ext2_umount("somewhere");

	return 0;
}
