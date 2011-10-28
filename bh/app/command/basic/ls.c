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
	struct block_device *block_dev;

	curr_volume = get_curr_volume();
	block_dev   = get_bdev_by_volume(curr_volume);
	if (block_dev == NULL)
		return -1;

	if (flag == 1) {
		putchar('\n');
		printf("volume:       %c:\n"
			   "name:         %s\n"
			   "start:        0x%08x\n"
			   "end:          0x%08x\n"
			   "size:         0x%08x\n",
				block_dev->volume,
				block_dev->dev.name,
				block_dev->bdev_base,
				block_dev->bdev_base + block_dev->bdev_size,
			    block_dev->bdev_size);

		if (!strncmp(block_dev->dev.name, "mtdblock", strlen("mtdblock"))) {
			flash = container_of(block_dev, struct flash_chip, bdev);
			printf("block size:   0x%08x\n", flash->oob_size);
			if (strchr(block_dev->dev.name, 'p'))
				printf("flash:        %s\n", flash->master->name);
			else
				printf("flash:        %s\n", flash->name);
		} else {
			disk = container_of(block_dev, struct disk_drive, bdev);
			printf("sector size:   %u\n", disk->sect_size);
		}
	} else {
		printf("\ndevice name: %s  (%c:)\n", block_dev->dev.name, block_dev->volume);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int opt;
	int ret = 0;
	int flag_l = 0;

	while ((opt = getopt(argc, argv, "l")) != -1) {
		switch (opt) {
		case 'l':
			flag_l = 1;
			break;
		default:
			usage();
			return -EINVAL;
		}
	}

	if ((flag_l == 0 && argc == 2) || argc > 2) {
		usage();
		return -EINVAL;
	}

	ret = show_info(flag_l);
	if (ret < 0)
		printf("Show device information failed!\n");

	return ret;
}
