#pragma once

#include <types.h>
#include <list.h>
#include <device.h>

#define LABEL_NAME_SIZE  32

#define BDEV_NAME_FLASH  "mtdblock"
#define BDEV_NAME_MMC    "mmcblock"
#define BDEV_NAME_SCSI   "sd"
#define BDEV_NAME_ATA    "hd" // fixme
#define BDEV_NAME_NBD    "nbd"

struct block_device;

struct part_attr {
	__u32 base;
	__u32 size;
	char  label[LABEL_NAME_SIZE];
};

struct block_buff {
	// __u32  blk_id;
	__u32  blk_size;
	__u8  *blk_base;
	__u8  *blk_off;
};

struct bdev_file {
	struct block_device *bdev;
	struct block_buff blk_buf;

	size_t cur_pos;
	const char *img_type;
	char name[FILE_NAME_SIZE];
	size_t size;

	int (*open)(struct bdev_file *, int);
	int (*close)(struct bdev_file *);

	ssize_t (*read)(struct bdev_file *, void *, size_t);
	ssize_t (*write)(struct bdev_file *, const void *, size_t);
};

struct block_device {
	// struct device dev;
	char name[MAX_DEV_NAME]; // mmcblockXpY, mtdblockN, sdaN

	// part_attr
	size_t base;
	size_t size;
	char   label[LABEL_NAME_SIZE];

	// fixme!
	void *fs;
	char volume;
	struct bdev_file *file;

	struct list_node bdev_node;
};

int block_device_register(struct block_device *bdev);

struct block_device *get_bdev_by_name(const char *name);
struct block_device *get_bdev_by_volume(char vol);
