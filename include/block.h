#pragma once

#include <types.h>
#include <list.h>
#include <device.h>

#define PART_NAME_LEN 32
#define MAX_FILE_NAME_LEN   64

#define BDEV_NAME_FLASH  "mtdblock"
#define BDEV_NAME_MMC    "mmcblock"
#define BDEV_NAME_SCSI   "sd"
#define BDEV_NAME_ATA    "hd" // fixme
#define BDEV_NAME_NBD    "nbd"

typedef enum {
	PT_NONE,
	PT_BAD,
	PT_FREE,
	PT_BL_GTH,
	PT_BL_GBH,
	PT_BL_GCONF,
	PT_BL_UBOOT,
	PT_OS_LINUX,
	PT_OS_WINCE,
	PT_FS_RAMDISK,
	PT_FS_CRAMFS,
	PT_FS_BEGIN = PT_FS_CRAMFS,
	PT_FS_JFFS2,
	PT_FS_YAFFS,
	PT_FS_YAFFS2,
	PT_FS_UBIFS,
	PT_FS_END = PT_FS_UBIFS
} PART_TYPE;

struct block_device;

struct part_attr {
	PART_TYPE part_type;
	__u32   part_base;
	__u32   part_size;
	char  part_name[PART_NAME_LEN];
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
	char name[MAX_FILE_NAME_LEN];
	size_t size;

	int (*open)(struct bdev_file *, const char *);
	int (*close)(struct bdev_file *);

	ssize_t (*read)(struct bdev_file *, void *, size_t);
	ssize_t (*write)(struct bdev_file *, const void *, size_t);
};

struct block_device {
	// struct device dev;
	char name[MAX_DEV_NAME]; // mmcblockXpY, mtdblockN, sdaN

	// part_attr
	size_t bdev_base;
	size_t bdev_size;
	char   label[PART_NAME_LEN];

	// fixme!
	void *fs;
	char volume;
	struct bdev_file *file;

	struct list_node bdev_node;
};

int block_device_register(struct block_device *bdev);

struct block_device *get_bdev_by_name(const char *name);
struct block_device *get_bdev_by_volume(char vol);
