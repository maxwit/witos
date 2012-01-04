#include <stdio.h>
#include <list.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <fs/fs.h>

#define BUF_LEN 512
#define MNTPT   "c"

int ext2_init(void);
int fat_init(void);

void *zalloc(size_t sz)
{
	void *p;

	p = malloc(sz);
	if (p)
		memset(p, 0, sz);

	return p;
}

int main(int argc, char *argv[])
{
	int ret, fd;
	ssize_t len;
	const char *disk, *type;
	char buff[BUF_LEN], part[BUF_LEN], path[BUF_LEN];

	if (argc != 5) {
		printf("usage:\n\t%s disk partno type file\n", argv[0]);
		return -EINVAL;
	}

	disk = argv[1];
	snprintf(part, BUF_LEN, "foo0p%s", argv[2]);
	type = argv[3];
	snprintf(path, BUF_LEN, MNTPT ":%s", argv[4]);

	// ret = foo_drv_init(disk);
	if (ret < 0)
		return ret;

	ret = ext2_init();
	if (ret < 0)
		return ret;

	ret = fat_init();
	if (ret < 0)
		return ret;

	// ret = gb_mount(type, 0, part, MNTPT);
	if (ret < 0) {
		printf("fail to mount %s with %s! (ret = %d)\n", part, type, ret);
		return ret;
	}

	// fd = gb_open(path, 0);
	if (fd < 0) {
		printf("fail to open %s\n", path);
		return fd;
	}

	// len = gb_read(fd, buff, BUF_LEN);

	if (len > 0) {
		buff[len] = '\0';
		printf("data:\n\t%s\n", buff);
	}
	printf("%s() line %d\n", __func__, __LINE__);

	// gb_close(fd);

	// gb_umount(MNTPT);

	return 0;
}
