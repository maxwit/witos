#pragma once

#include <types.h>
#include <list.h>
#include <device.h>

#define LABEL_NAME_SIZE  32

#define BDEV_NAME_FLASH  "mtdblock"
#define BDEV_NAME_MMC    "mmcblk"
#define BDEV_NAME_SCSI   "sd"
#define BDEV_NAME_ATA    "hd" // fixme
#define BDEV_NAME_NBD    "nbd"

#define BDF_RDONLY       (1 << 0)

struct block_device;

struct part_attr {
	__u32 base;
	__u32 size;
	char  label[LABEL_NAME_SIZE];
};

struct block_device {
	// struct device dev;
	char name[MAX_DEV_NAME]; // mmcblockXpY, mtdblockN, sdaN

	__u32 flags;

	// part_attr
	size_t base;
	size_t size;
	char   label[LABEL_NAME_SIZE];

	// fixme: to be moved here from lower layer
	// struct block_device *master;
	// struct list_node slave_list;
	// ...

	// fixme: to be removed
	struct file_system *fs;
	const struct file_operations *fops;

	struct list_node bdev_node;
};

int block_device_register(struct block_device *bdev);
