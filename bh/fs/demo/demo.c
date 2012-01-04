#include <stdio.h>
#include <getopt.h>
#include <list.h>
#include <errno.h>
#include <demofs.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <fs/fs.h>

#define BUF_LEN 511
#define MNTPT   "c"

int main(int argc, char *argv[])
{
	int ret, fd;
	ssize_t len;
	const char *dev_name = "/tmp/hello.ext2", *type = "ext2", *path = "/" MNTPT "/"  __FILE__;
	char buff[BUF_LEN + 1];
	struct block_device bdev;

	while ((ret = getopt(argc, argv, "d:t:f:h")) != -1) {
		switch (ret) {
		case 'd':
			dev_name = optarg;
			break;

		case 't':
			type = optarg;
			break;

		case 'f':
			path = optarg;
			break;

		default:
			break;
		}
	}

	strcpy(bdev.name, dev_name);
	block_device_register(&bdev);

	ret = demo_mount(type, 0, dev_name, MNTPT);
	if (ret < 0) {
		printf("fail to mount %s with %s! (ret = %d)\n", dev_name, type, ret);
		return ret;
	}

	fd = demo_open(path, 0);
	if (fd < 0) {
		printf("fail to open %s\n", path);
		return fd;
	}

	while (1) {
		len = demo_read(fd, buff, BUF_LEN);
		if (len <= 0)
			break;

		buff[len] = '\0';
		printf("%s", buff);
	}

	printf("%s() line %d\n", __func__, __LINE__);

	demo_close(fd);

	demo_umount(MNTPT);

	return 0;
}
