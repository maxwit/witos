#include <string.h>
#include <block.h>
#include <shell.h>

#if 0
#define IS_ALPH(c) (((c) >= 'a' && (c) <= 'z') || \
			((c) >= 'A' && (c) <= 'Z'))

#define STR_LEN  8

static void block_info_show(struct block_device *bdev)
{
	char hr_size[STR_LEN];

	val_to_hr_str(bdev->size, hr_size);

	printf("0x%08x - 0x%08x %s: %s (%s) %s\n",
		bdev->base, bdev->base + bdev->size,
		hr_size, bdev->name, bdev->label);
}
#endif

// fixme
int main(int argc, char *argv[])
{
	char v;
	int index;
	const char *vol;
	struct block_device *bdev;

	switch (argc) {
	case 1:
		v = get_home_volume();
		bdev = get_bdev_by_volume(v);
		break;

	case 2:
		vol = argv[1];

		if (dec_str_to_val(vol, &index) >= 0 && index > 0)
			bdev = get_bdev_by_volume('A' + index - 1);
		else
			bdev = get_bdev_by_name(vol);

		break;

	default:
		usage();
		return -EINVAL;
	}

	if (!bdev) {
		usage();
		return -ENODEV;
	}

	// block_info_show(bdev);

	v = bdev->volume;
	if (v >= 'a' && v <= 'z')
		v = v + 'A' -'a';

	set_curr_volume(v);

	return 0;
}
