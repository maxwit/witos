#include <block.h>
#include <shell.h>
#include <drive.h>
#include <getopt.h>
#include <flash/flash.h>

static int show_info(int verbose)
{
	char vol;
	struct disk_drive *disk;
	struct flash_chip *flash;
	struct block_device *bdev;

	vol = get_curr_volume();
	bdev = get_bdev_by_volume(vol);
	if (bdev == NULL)
		return -ENODEV;

	if (verbose == 1) {
		putchar('\n');
		printf("label:        %s (%c)\n"
			   "device:       %s\n"
			   "start:        0x%08x\n"
			   "end:          0x%08x\n"
			   "size:         0x%08x\n",
				bdev->label[0] ? bdev->label : "N/A", bdev->volume,
				bdev->name,
				bdev->bdev_base,
				bdev->bdev_base + bdev->bdev_size,
			    bdev->bdev_size);

		if (!strncmp(bdev->name, BDEV_NAME_FLASH, sizeof(BDEV_NAME_FLASH) - 1)) {
			flash = container_of(bdev, struct flash_chip, bdev);
			printf("block size   : 0x%08x\n"
				   "page size    : 0x%08x\n"
				   "host         : %s\n",
				   flash->erase_size,
				   flash->write_size,
				   flash->master->name); // fixme: in case of no part available
		} else {
			disk = container_of(bdev, struct disk_drive, bdev);
			printf("sector size:   %u\n", disk->sect_size);
		}
	} else {
		printf("\ndevice name: %s  (%c:)\n", bdev->name, bdev->volume);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int opt;
	int ret = 0;
	int verbose = 0;

	while ((opt = getopt(argc, argv, "l")) != -1) {
		switch (opt) {
		case 'l':
			verbose = 1;
			break;
		default:
			usage();
			return -EINVAL;
		}
	}

	if ((verbose == 0 && argc == 2) || argc > 2) {
		usage();
		return -EINVAL;
	}

	ret = show_info(verbose);

	return ret;
}
