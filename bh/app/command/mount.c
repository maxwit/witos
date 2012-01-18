#include <stdio.h>
#include <errno.h>
#include <fs/fs.h>
#include <fcntl.h>

extern int list_mount();

int main(int argc, char *argv[])
{
	int ret;
	const char *bdev, *type, *path;

	if (argc == 1) {
		list_mount();
		return 0;
	}

	if (argc != 5) {
		printf("Usage:\n\t%s -t type device path\n", argv[0]);
		return -EINVAL;
	}

	type = argv[2];
	bdev = argv[3];
	path = argv[4];

	ret = mount(bdev, path, type, 0);
	if (ret < 0)
		printf("fail to mount %s (ret = %d)\n", bdev, ret);

	return ret;
}
