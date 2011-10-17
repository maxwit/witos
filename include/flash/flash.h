#pragma once

#include <types.h>
#include <list.h>
#include <block.h>

// #define MTD_DEV_NAME_LEN    32

#define MAX_FILE_NAME_LEN   64
#define MAX_FLASH_PARTS	    16
// #define DEF_VOL_ID          1  // fixme: default point to g-bios bottom half
#define BOOT_FLASH_ID       0 // fixme

#define FLASH_ABSENT		0
#define FLASH_RAM			1
#define FLASH_ROM			2
#define FLASH_NORFLASH		3
#define FLASH_NANDFLASH		4
#define FLASH_DATAFLASH		6
#define FLASH_UBIVOLUME		7
#define FLASH_FLASH_PART   (1 << 6) // fixme

enum
{
	FLASH_IOCS_OOB_MODE,
	FLASH_IOCG_OOB_MODE,
	FLASH_IOCS_CALLBACK,
	FLASH_IOC_SCANBB,
};

#define BLOCK_DEV_NAME_LEN  32

#define IS_FS_PART(type) (PT_FS_BEGIN <= (type) && (type) <= PT_FS_END)

struct flash_chip;

struct oob_free_region
{
	u32 nOfOffset;
	u32 nOfLen;
};

#define FLASH_MAX_OOBFREE_ENTRIES	8

struct nand_oob_layout
{
	u32 ecc_code_len;
	u32 ecc_pos[64];

	u32 free_oob_sum;
	struct oob_free_region free_region[FLASH_MAX_OOBFREE_ENTRIES];
};

struct ecc_stats
{
	u32 nEccCorrectCount;
	u32 nEccFailedCount;
	u32 badblocks;
	u32 bbtblocks;
};

typedef struct
{
	u32 nPageIndex;
	u32 nBlockIndex;
} FLASH_HOOK_PARAM;

typedef int (*FLASH_HOOK_FUNC)(struct flash_chip *, FLASH_HOOK_PARAM *);

typedef struct
{
	FLASH_HOOK_PARAM *args;
	FLASH_HOOK_FUNC func;
} FLASH_CALLBACK;

#define MAX_FLASH_DEVICES  32

#define FLASH_ERASE_PENDING 0x01
#define FLASH_ERASING		0x02
#define FLASH_ERASE_SUSPEND	0x04
#define FLASH_ERASE_DONE    0x08
#define FLASH_ERASE_FAILED  0x10

#define MAX_ECC_STEPS  16

// fixme!
#define EDF_NORMAL     0
#define EDF_JFFS2      (1 << 0)
#define EDF_ALLOWBB    (1 << 1)

struct erase_opt
{
	u32 estart;
	u32 esize;
	int for_jffs2;
	int bad_allow;
	u32 fail_addr;
	u8  estate;
};

typedef enum
{
	FLASH_OOB_PLACE,
	FLASH_OOB_AUTO,
	FLASH_OOB_RAW,
} OOB_MODE;

struct oob_opt
{
	u8  *data_buff;
	u8	*oob_buff;
	u32	 data_len;
	u32  oob_len;
	u32	 ret_len;
	u32  oob_ret_len;
	u32  oob_off;
	OOB_MODE  op_mode;
};

struct image_info
{
	char  image_name[MAX_FILE_NAME_LEN];
	u32   image_size;
};

struct block_buff
{
	u32   blk_id;
	u32   blk_size;
	u8   *blk_base;
	u8   *blk_off;
};

struct partition
{
	u32  cur_pos;

	struct flash_chip  *host;
	struct part_attr   *attr;
	struct image_info  *image;
	struct block_buff   blk_buf;
};

struct flash_chip
{
	struct block_device bdev;

	union
	{
		struct list_node master_node;
		struct list_node slave_node;
	};

	union
	{
		struct list_node slave_list;
		struct flash_chip *master;
	};
///////////////////////////////////
	int   type;
	char  name[BLOCK_DEV_NAME_LEN];

	size_t write_size;
	size_t erase_size;
	size_t chip_size; // fixme

	u32 write_shift;
	u32 erase_shift;
	u32 chip_shift;

	u32 oob_size;

	struct ecc_stats eccstat;

	int bad_allow;

	FLASH_HOOK_PARAM *callback_args;
	FLASH_HOOK_FUNC   callback_func;

	struct partition  part_tab[MAX_FLASH_PARTS];
	struct part_info  *pt_info;

	// struct part_attr  *conf_attr; // fixme: should be cached???

	int (*read)(struct flash_chip *, u32, u32, u32 *, u8 *);
	int (*write)(struct flash_chip *, u32, u32 , u32 *, const u8 *);
	int (*erase)(struct flash_chip *, struct erase_opt *);

	int (*read_oob)(struct flash_chip *, u32, struct oob_opt *);
	int (*write_oob)(struct flash_chip *,u32, struct oob_opt *);

	int (*block_is_bad)(struct flash_chip *, u32);
	int (*block_mark_bad)(struct flash_chip *, u32);
	int (*scan_bad_block)(struct flash_chip *);
	// #ifdef CONFIG_SUPPORT_LINUX
	int (*get_mtd_name)(const struct flash_chip *, char []);

	// fixme all! (not only for multi-thread)
	OOB_MODE oob_mode;
};

static u32 __INLINE__ flash_write_is_align(struct flash_chip *flash, u32 size)
{
	return (size + flash->write_size - 1) & ~(flash->write_size - 1);
}

static u32 __INLINE__ flash_erase_is_align(struct flash_chip *flash, u32 size)
{
	return (size + flash->erase_size - 1) & ~(flash->erase_size - 1);
}

int flash_register(struct flash_chip *flash);

int flash_unregister(struct flash_chip *flash);

struct flash_chip *flash_get(unsigned int num);

struct flash_chip *flash_get_by_name(const char *name);

const char *flash_get_mtd_name(const struct flash_chip *flash);

void __INIT__ flash_add_part_tab(const struct part_attr *attr, int num);

BOOL check_image_type(PART_TYPE type, const u8 *data);

typedef enum
{
	NAND_ECC_NONE,
	NAND_ECC_SW,
	NAND_ECC_HW,
//	NAND_ECC_YAFFS,
	NAND_ECC_YAFFS2,
} ECC_MODE;

int flash_set_ecc_mode(struct flash_chip *flash, ECC_MODE newMode, ECC_MODE *pOldMode);

// APIs
struct flash_chip *flash_open(unsigned int num);

int flash_close(struct flash_chip *flash);

int flash_read(struct flash_chip *flash, void *buf, int start, int count);

long flash_write(struct flash_chip *flash, const void *buf, u32 count, u32 ppos);

int flash_erase(struct flash_chip *flash, u32 nStartAddr, u32 nLen, u32 flags);

int flash_ioctl(struct flash_chip *flash, int cmd, void *arg);
