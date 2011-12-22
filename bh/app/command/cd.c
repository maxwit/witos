#include <string.h>
#include <block.h>
#include <shell.h>

#define IS_ALPH(c) (((c) >= 'a' && (c) <= 'z') || \
			((c) >= 'A' && (c) <= 'Z'))

#define STR_LEN  8
#define TAILL_NUM 239

static void block_info_show(struct block_device *bdev)
{
	char hr_size[STR_LEN];

	val_to_hr_str(bdev->bdev_size, hr_size);

	printf("0x%08x - 0x%08x %s: %s (%c) %s\n",
		bdev->bdev_base, bdev->bdev_base + bdev->bdev_size,
		hr_size, bdev->name, bdev->volume);
}

int main(int argc, char *argv[])
{
	char v;
	const char *vol;
	struct block_device *bdev;

	switch (argc) {
	case 1:
		v = get_home_volume();
		bdev = get_bdev_by_volume(v);
		break;

	case 2:
		vol = argv[1];

		if (IS_ALPH(vol[0]) && '\0' == vol[1])
			bdev = get_bdev_by_volume(vol[0]);
		else
			bdev = get_bdev_by_name(vol);

		break;

	default:
		usage();
		return -EINVAL;
	}

	if (!bdev) {
		printf("fail to cd to \"%c\", no such block device!\n", v);
		usage();
		return -ENODEV;
	}

	block_info_show(bdev);

	v = bdev->volume;
	if (v >= 'a' && v <= 'z')
		v = v + 'A' -'a';

	set_curr_volume(v);

	return 0;
}
