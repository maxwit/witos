#pragma once

#define GTH_MAGIC          (('G' << 24) | ('B' << 16) | (('t' - 'a') << 8) | 'h')
#define GTH_MAGIC_OFFSET    32

#define GBH_MAGIC          (('G' << 24) | ('B' << 16) | (('b' - 'a') << 8) | 'h')
#define GBH_MAGIC_OFFSET    32

#define G_SYS_MAGIC        ('G' << 24 | 'B' << 16 | 's' << 8 | 'c')
#define G_SYS_MAGIC_OFFSET  32

#define GBH_SIZE_OFFSET     20

#define LINUX_MAGIC         0x016f2818
#define LINUX_MAGIC_OFFSET  0x24

#define CRAMFS_MAGIC        0x28cd3d45

#define JFFS2_MAGIC         0x1985
#define JFFS2_MAGIC_OFFSET  0x0

#define UBIFS_MAGIC         0x06101831
#define UBIFS_MAGIC_OFFSET  0x0

#define YAFFS_OOB_SIZE		16
#define YAFFS2_OOB_SIZE		64
