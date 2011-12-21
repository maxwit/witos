#include <stdio.h>
#include <errno.h>
#include <fs/fs.h>

int main(int argc, char *argv[])
{
	int ret;
	const char *bdev, *type, *path;

	if (argc != 5) {
		printf("Usage:\n\t%s -t type device path\n", argv[0]);
		return -EINVAL;
	}

	type = argv[2];
	bdev = argv[3];
	path = argv[4];

	ret = mount(type, 0, bdev, path);
	if (ret < 0)
		printf("fail to mount %s (ret = %d)\n", bdev, ret);

	return ret;
}
