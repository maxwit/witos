#include <unistd.h>
#include <fcntl.h>
#include <block.h>

static int show_info(int verbose)
{
	int fd, ret;
	const char *cwd;
	struct part_attr part;

	cwd = getcwd();
	fd = open(cwd, O_RDONLY);

	ret = ioctl(fd, 0 /* fixme */, &part);

	close(fd);
#if 0
	if (verbose == 1) {
		printf("device      : %s\n"
			   "label       : %s\n"
			   "start       : 0x%08x\n"
			   "end         : 0x%08x\n"
			   "size        : 0x%08x\n",
				bdev->name,
				bdev->label[0] ? bdev->label : "N/A",
				bdev->base,
				bdev->base + bdev->size,
			    bdev->size);

		if (!strncmp(bdev->name, BDEV_NAME_FLASH, strlen(BDEV_NAME_FLASH))) {
			flash = container_of(bdev, struct flash_chip, bdev);
			printf("block size  : 0x%08x\n"
				   "page size   : 0x%08x\n"
				   "host        : %s\n",
				   flash->erase_size,
				   flash->write_size,
				   flash->master->name); // fixme: in case of no part available
		} else {
			disk = container_of(bdev, struct disk_drive, bdev);
			printf("sector size:   %u\n", disk->sect_size);
		}
	} else {
		printf("\ndevice: %s (%s)\n", bdev->name, bdev->label);
	}
#endif

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

	if (optind < argc) {
		usage();
		return -EINVAL;
	}

	ret = show_info(1);

	return ret;
}
