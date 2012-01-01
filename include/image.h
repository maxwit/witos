#pragma once

#define LINUX_MAGIC         0x016f2818
#define LINUX_MAGIC_OFFSET  0x24

#define CRAMFS_MAGIC        0x28cd3d45

#define JFFS2_MAGIC         0x1985
#define JFFS2_MAGIC_OFFSET  0x0

#define UBIFS_MAGIC         0x06101831
#define UBIFS_MAGIC_OFFSET  0x0

#define YAFFS_OOB_SIZE		16
#define YAFFS2_OOB_SIZE		64

#define CHECK_IMG_MINSIZE   128

// image
typedef enum {
	IMG_UNKNOWN,
	// Bootloader
	IMG_GTH,
	IMG_GBH,
	IMG_UEFI,
	IMG_UBOOT,
	// OS
	IMG_LINUX,
	IMG_WITOS,
	IMG_MSWIN,
	IMG_ANDROID,
	// FS
	IMG_INITRD,
	IMG_JFFS2,
	IMG_YAFFS1,
	IMG_YAFFS2,
	IMG_CRAMFS,
	IMG_UBIFS,
	IMG_NTFS,
	IMG_FAT,
	// ...
	IMG_MAX
} image_t;

image_t image_type_detect(const void *data, size_t size);
