#include <string.h>
#include <block.h>
#include <shell.h>

#define IS_ALPH(c) (((c) >= 'a' && (c) <= 'z') || \
			((c) >= 'A' && (c) <= 'Z'))

#define STR_LEN  8
#define TAILL_NUM 239

static void block_info_show(struct block_device *bdev)
{
	__u32 device_size = bdev->bdev_size;
	char str[STR_LEN];

	if (device_size >= (1 << 30)) {
		snprintf(str, STR_LEN, "%d.%dG",
			device_size >> 30, (((device_size >> 20) & 0x3FF) * 10) >> 10);
	} else if (device_size >= (1 << 20)) {
		snprintf(str, STR_LEN, "%d.%dM",
			device_size >> 20, (((device_size >> 10) & 0x3FF) * 10) >> 10);
	} else if (device_size >= (1 << 10)) {
		snprintf(str, STR_LEN, "%d.%dK",
			device_size >> 10, ((device_size & 0x3FF) * 10) >> 10);
	} else {
		snprintf(str, STR_LEN, "%d", device_size);
	}

	printf("0x%08x - 0x%08x %s (%c:) --- %s bytes\n",
		bdev->bdev_base, bdev->bdev_base + bdev->bdev_size,
		bdev->dev.name, bdev->volume, str);
}

int main(int argc, char *argv[])
{
	char v;
	const char *vol;
	struct block_device *bdev;

	switch (argc) {
	case 1:
		v = get_home_volume();
		break;

	case 2:
		vol = argv[1];

		if (IS_ALPH(vol[0]) && \
			('\0' == vol[1] || (':' == vol[1] && '\0' == vol[2])))
				v = vol[0];
		else {
			usage();
			return -EINVAL;
		};
		break;

	default:
		usage();
		return -EINVAL;
	}

	bdev = get_bdev_by_volume(v);
	if (!bdev) {
		printf("fail to change to \"%c\", no such block device!\n", v);
		return -ENODEV;
	}

	block_info_show(bdev);

	if (v >= 'a' && v <= 'z') {
		v = v + 'A' -'a';
	}

	set_curr_volume(v);

	return 0;
}
