#include <errno.h>
#include <mmc/mmc.h>
#include <mmc/mmc_ops.h>
#include <unistd.h>
#include <string.h>
#include <task.h>
#include <types.h>
#include <drive.h>
#include <block.h>

#define BUFF_LEN            512
#define BLK_LEN             512
#define MMC_HOST_NUM        5
#define DEFAULT_UNIT_SIZE   1

#if 0
static int scan(int argc, char *argv[])
{
	int	ret;
	int i;
	struct mmc_cid *cid;
	struct mmc_host *host;

	if (argc > 1) {
		usage();

		return -EINVAL;
	}

	for (i = 0; i < MMC_HOST_NUM; i++) {
		host = mmc_get_host(i);
		if(host == NULL && i == 0) {
			printf("Error: Can't Get MMC Host!\n");
			return -ENODEV;
		}

		if (host == NULL) {
			printf("Scan complete\n");
			break;
		}

		ret = mmc_sd_detect_card(host);
		if (ret < 0) {
			printf("Error: No SD Found!\n");
			return -ENODEV;
		}

		cid = (struct mmc_cid* )(host->card.raw_cid);

		printf("Manufacturer ID: %x\n"
				"OEM/Application ID: %x\n"
				"Product Name: %c%c%c%c%c\n"
				"Product serial number: %x\n",
				cid->mid, cid->oid,cid->pnm[4],
				cid->pnm[3],cid->pnm[2],cid->pnm[1],cid->pnm[0],
				cid->psn);
	}

	return 0;
}

static int dump(int argc, char *argv[])
{
	int opt;
	int flag = 0;
	__u8 buf[BLK_LEN];
	int ret = 0;
	__u32 addr = 0;
	__u32 unit_size;
	struct block_device *bdev;
	struct disk_drive *disk;
	char vol;
	int i, j;

	while ((opt = getopt(argc, argv, "a::hw:")) != -1) {
		switch (opt) {
		case 'a':
			if (optarg != NULL) {
				ret = str_to_val(optarg, (unsigned long *)&addr);
				if (ret < 0) {
					printf("Error: Address error!\n");

					return -EINVAL;
				}
			}

			printf("Dump from the address: 0x%x\n", addr);
			break;

		case 'w':
			if (optarg != NULL) {
				ret = str_to_val(optarg, (unsigned long *)&unit_size);
				if (ret < 0){
					printf("Error: Unit_size error!\n");
					return -EINVAL;
				}
			}

			if (!(unit_size == 1 || unit_size == 2 || unit_size == 4 || unit_size == 8)) {
				printf("Error: invalid unit_size!\n");
				return -EINVAL;
			}

			flag = 1;
			printf("Unit_size: 0x%x\n", unit_size);
			break;

		case 'h':
		default:
			usage();
			return 0;
		}
	}

	if (flag == 0){
		unit_size = DEFAULT_UNIT_SIZE;
	}

	// vol = get_home_volume();

	// bdev = get_bdev_by_index(vol);
	if (!bdev) {
		printf("fail to change to \"%c\", no such block device!\n", vol);
		return -ENODEV;
	}

	disk = container_of(bdev, struct disk_drive, bdev);

	memset(buf, 0xbb, BLK_LEN);
	printf("get_block: %p\n", disk->get_block);

	ret = disk->get_block(disk, addr, buf);
	if (ret < 0) {
		printf("Error: Fail to Read!\n");
		return ret;
	}

	for (i = 0; i < BLK_LEN / unit_size; i++) {
		for (j = 0; j < unit_size; j++) {
			printf("%0x", ((__u8*)buf)[i * unit_size + i]);
		}

		if ((i + 1)% 8) {
			printf(" ");
		} else {
			printf("\n");
		}
	}

	return 0;
}

static int write(int argc, char *argv[])
{
	int i;
	int opt;
	int ret = 0;
	char volume;
	__u8 buf[BLK_LEN];
	__u32 mem_addr = 0, disk_addr = 0;
	struct block_device *bdev;
	struct disk_drive *drive;

	while ((opt = getopt(argc, argv, "a::m:h")) != -1) {
		switch (opt) {
		case 'a':
			if (optarg != NULL) {
				ret = str_to_val(optarg, (unsigned long *)&disk_addr);
				if (ret < 0) {
					printf("Error: Start disk address error!\n");

					return -EINVAL;
				}
			}

			printf("Disk address: 0x%x\n", disk_addr);
			break;

		case 'm':
			if (optarg != NULL) {
				ret = str_to_val(optarg, (unsigned long *)&mem_addr);
				if (ret < 0) {
					printf("Error: Start memory address error!\n");

					return -EINVAL;
				}
			}

			printf("Memory address: 0x%x\n", mem_addr);
			break;

		case 'h':
		default:
			usage();

			return 0;
		}
	}

	// volume = get_home_volume();

	printf("volume: %c\n", volume);

	// bdev = get_bdev_by_index(volume);
	if (!bdev) {
		printf("fail to get bdev \"%c\", no such block device!\n", volume);

		return -ENODEV;
	}

	drive = container_of(bdev, struct disk_drive, bdev);

	for (i = 0; i < BLK_LEN; i++) {
		buf[i] = *(unsigned char *)mem_addr;
		mem_addr++;
	}

	printf("drive->put_block() address:%p\n", drive->put_block);

	ret = drive->put_block(drive, disk_addr, buf);
	if (ret < 0) {
		printf("Write failed!\n");
	}

	printf("Write to disk sucess!\n");

	return 0;
}

static int read(int argc, char *argv[])
{
	int i;
	int opt;
	int ret = 0;
	char volume;
	__u8 buf[BLK_LEN];
	__u32 mem_addr = 0, disk_addr = 0;
	struct block_device *bdev;
	struct disk_drive *drive;

	while ((opt = getopt(argc, argv, "a::m:h")) != -1) {
		switch (opt) {
		case 'a':
			if (optarg != NULL) {
				ret = str_to_val(optarg, (unsigned long *)&disk_addr);
				if (ret < 0) {
					printf("Error: Start disk address error!\n");

					return -EINVAL;
				}
			}

			printf("Disk address: 0x%x\n", disk_addr);
			break;

		case 'm':
			if (optarg != NULL) {
				ret = str_to_val(optarg, (unsigned long *)&mem_addr);
				if (ret < 0) {
					printf("Error: Start memory address error!\n");

					return -EINVAL;
				}
			}

			printf("Memory address: 0x%x\n", mem_addr);
			break;

		case 'h':
		default:
			usage();

			return 0;
		}
	}

	// volume = get_home_volume();

	printf("volume: %c\n", volume);

	// bdev = get_bdev_by_index(volume);
	if (!bdev) {
		printf("fail to get bdev \"%c\", no such block device!\n", volume);

		return -ENODEV;
	}

	drive = container_of(bdev, struct disk_drive, bdev);

	printf("drive->get_block address: %p\n", drive->get_block);

	ret = drive->get_block(drive, disk_addr, buf);
	if (ret < 0) {
		printf("Read failed!\n");
	}

	for (i = 0; i < BLK_LEN; i++) {
		*(unsigned char *)mem_addr = buf[i];
		mem_addr++;
	}

	printf("Read disk sucess!\n");

	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	struct command cmd[] = {
		{
			.name = "scan",
			.main = scan
		},
		{
			.name = "dump",
			.main = dump
		},
		{
			.name = "write",
			.main = write
		},
		{
			.name = "read",
			.main = read
		}
	};

	if (argc >= 2) {
		for (i = 0; i < ARRAY_ELEM_NUM(cmd); i++) {
			if (strcmp(argv[1], cmd[i].name) == 0) {
				cmd[i].main(argc - 1, argv + 1);

				return 0;
			}
		}
	}

	usage();

	return -1;
 }
#else
int main()
{return 0;}
#endif
