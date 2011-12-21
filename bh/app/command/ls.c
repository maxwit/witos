#include <block.h>
#include <shell.h>
#include <drive.h>
#include <getopt.h>
#include <flash/flash.h>

static int show_info(int flag)
{
	char curr_volume;
	struct disk_drive *disk;
	struct flash_chip *flash;
	struct block_device *bdev;

	curr_volume = get_curr_volume();
	bdev = get_bdev_by_volume(curr_volume);
	if (bdev == NULL)
		return -1;

	if (flag == 1) {
		putchar('\n');
		printf("volume:       %c:\n"
			   "name:         %s\n"
			   "start:        0x%08x\n"
			   "end:          0x%08x\n"
			   "size:         0x%08x\n",
				bdev->volume,
				bdev->name,
				bdev->bdev_base,
				bdev->bdev_base + bdev->bdev_size,
			    bdev->bdev_size);

		if (!strncmp(bdev->name, "mtdblock", strlen("mtdblock"))) {
			flash = container_of(bdev, struct flash_chip, bdev);
			printf("block size:   0x%08x\n"
				   "page size:    0x%08x\n",
				   flash->erase_size, flash->write_size);
			if (strchr(bdev->name, 'p'))
				printf("flash:        %s\n", flash->master->name);
			else
				printf("flash:        %s\n", flash->name);
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
	int flag = 0;

	while ((opt = getopt(argc, argv, "l")) != -1) {
		switch (opt) {
		case 'l':
			flag = 1;
			break;
		default:
			usage();
			return -EINVAL;
		}
	}

	if ((flag == 0 && argc == 2) || argc > 2) {
		usage();
		return -EINVAL;
	}

	ret = show_info(flag);

	return ret;
}
