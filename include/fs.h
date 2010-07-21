#pragma once


typedef unsigned int FS_TYPE;

#define FS_TYPE_GENERAL     1
#define FS_TYPE_JFFS2       2
#define FS_TYPE_YAFFS       3
#define FS_TYPE_YAFFS2      4


#define LINUX_MAGIC    0x016f2818
#define CRAMFS_MAGIC   0x28cd3d45
#define JFFS2_MAGIC    0x1985
#define UBIFS_MAGIC    0x06101831

#define LINUX_MAGIC_OFFSET	0x24
#define JFFS2_MAGIC_OFFSET  0x0
#define UBIFS_MAGIC_OFFSET  0x0

#define O_CREAT   (1)
#define O_RDWR    (1)

int close(int fd);

struct image_cache *image_cache_get(PART_TYPE type);

