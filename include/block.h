#pragma once

#include <types.h>
#include <list.h>
#include <device.h>

#define PART_NAME_LEN 32

typedef enum
{
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

struct part_attr
{
	PART_TYPE part_type;
	u32   part_base;
	u32   part_size;
	char  part_name[PART_NAME_LEN];
};

struct bdev_ops {
	int (*get_blk)(struct block_device *, int, __u8 *);
	int (*put_blk)(struct block_device *, int, __u8 *);
};

struct block_device
{
	struct device dev;

	size_t bdev_base;
	size_t bdev_size;

	// fixme!
	void *fs;
	char volume;
	const struct bdev_ops *ops;

	struct list_node bdev_node;
};

int block_device_register(struct block_device *bdev);

struct block_device *get_bdev_by_name(const char *name);
struct block_device *get_bdev_by_volume(char vol);
