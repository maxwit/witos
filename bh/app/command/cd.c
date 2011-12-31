#include <string.h>
#include <unistd.h>
#include <block.h> // fixme

int main(int argc, char *argv[])
{
	int index, ret;
	const char *dir;
	char home[CONF_VAL_LEN];
	struct block_device *bdev;

	switch (argc) {
	case 1:
		ret = conf_get_attr("HOME", home);
		assert(ret <= 0); // should never fail!
		dir = home;
		break;

	case 2:
		ret = dec_str_to_val(argv[1], &index);
		if (ret >= 0) {
			if (index >= 0) {
				bdev = get_bdev_by_index(index);
				if (bdev) {
					dir = bdev->name;
					break;
				}
			}

			usage();
			return -ENOENT;
		}

		dir = argv[1];
		break;

	default:
		usage();
		return -EINVAL;
	}

	ret = chdir(dir);
	if (ret < 0) {
		printf("cd failed, errno = %d!\n", ret);
		return ret;
	}

	return ret;
}
