#include <stdio.h>
#include <drive.h>
#include "fat.h"

#define LEN 512

int perror(const char *);

int block_device_init();
int disk_drive_init();
int foo_drv_init(const char *);

int fat_fs_init(void);

int mount(const char *type, unsigned long flags, const char *dev, const char *path);

int main(int argc, char *argv[])
{
	int ret;
	const char *dev, *fn;
	struct fat_file *fp;
	char buff[LEN];

	dev = (argc >= 2) ? argv[1] : "hd.img";
	fn = (3 == argc) ? argv[2] : "abc.s";

	// block device init
	block_device_init();
	disk_drive_init();
	foo_drv_init(dev);

	// fat fs init
	fat_fs_init();

	// mount
	ret = mount("vfat", 0, "foo0p1", 0);
	if (ret < 0)
	{
		printf("fail to mount foo0p1! (ret = %d)\n", ret);
		return ret;
	}

	// read and write
	sprintf(buff, "foo0p1:%s", fn);
	fp = fat_open("foo0p1:abc.s", 0);
	if (NULL == fp)
	{
		printf("fail to open xx!\n");
		return -1;
	}

	fat_read(fp, buff, LEN);

	printf("%s\n", buff);

	return 0;
}
