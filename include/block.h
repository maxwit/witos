#pragma once
#include <device.h>

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

#define PART_NAME_LEN 32

struct part_attr
{
	PART_TYPE part_type;
	u32   part_base;
	u32   part_size;
	char  part_name[PART_NAME_LEN];
};

struct block_device
{
	struct device dev;

	u32  part_base;
	u32  part_size;
};

int block_device_register(struct block_device *blk_dev);
