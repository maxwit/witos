#pragma once

#include <types.h>
#include <list.h>
#include <block.h>

// #define MTD_DEV_NAME_LEN    32

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

enum {
	FLASH_IOCG_INFO,
	FLASH_IOCS_OOB_MODE,
	FLASH_IOCG_OOB_MODE,
	FLASH_IOCS_CALLBACK,
	FLASH_IOCG_SIZE,
	FLASH_IOC_SCANBB,
	FLASH_IOC_ERASE,
};

#define BLOCK_DEV_NAME_LEN  32
#define MTD_ID_NAME_LEN     32

#define IS_FS_PART(type) (IMG_BEGIN <= (type) && (type) <= IMG_END)

struct flash_chip;

struct oob_free_region {
	__u32 nOfOffset;
	__u32 nOfLen;
};

#define FLASH_MAX_OOBFREE_ENTRIES	8

struct nand_oob_layout {
	__u32 ecc_code_len;
	__u32 ecc_pos[64];

	__u32 free_oob_sum;
	struct oob_free_region free_region[FLASH_MAX_OOBFREE_ENTRIES];
};

struct ecc_stats {
	__u32 ecc_correct_count;
	__u32 ecc_failed_count;
	__u32 badblocks;
	__u32 bbtblocks;
};

typedef struct {
	__u32 page_index;
	__u32 block_index;
} FLASH_HOOK_PARAM;

typedef int (*FLASH_HOOK_FUNC)(struct flash_chip *, FLASH_HOOK_PARAM *);

typedef struct {
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

// Flash erase flags
#define EDF_NORMAL     0
#define EDF_JFFS2      (1 << 0)
#define EDF_ALLOWBB    (1 << 8)

struct erase_opt {
	__u32 estart;
	__u32 esize;
	__u32 flags;
	__u32 fail_addr; // fail_at, faddr
	__u8  estate;
};

typedef enum {
	FLASH_OOB_PLACE,
	FLASH_OOB_AUTO,
	FLASH_OOB_RAW,
} OOB_MODE;

struct oob_opt {
	__u8  *data_buff;
	__u8  *oob_buff;
	__u32  data_len;
	__u32  oob_len;
	__u32  ret_len;
	__u32  oob_ret_len;
	__u32  oob_off;
	OOB_MODE  op_mode;
};

struct image_info {
	char  image_name[FILE_NAME_SIZE];
	__u32 image_size;
};

struct flash_info {
	int type;
	size_t block_size;
	size_t page_size;
	size_t oob_size;
	OOB_MODE oob_mode;
	const char *name;
	size_t bdev_base;
	size_t bdev_size;
	const char *bdev_label;
};

struct flash_chip {
	struct block_device bdev;

	union {
		struct list_node master_node;
		struct list_node slave_node;
	};

	union {
		struct list_node slave_list;
		struct flash_chip *master;
	};

	int   type;
	char  name[MTD_ID_NAME_LEN]; // fixme: no need for slaves

	size_t write_size;
	size_t erase_size;
	size_t chip_size; // fixme

	__u32 write_shift;
	__u32 erase_shift;
	__u32 chip_shift;

	__u32 oob_size;

	struct ecc_stats eccstat;

	int bad_allow;

	FLASH_HOOK_PARAM *callback_args;
	FLASH_HOOK_FUNC   callback_func;

	int (*read)(struct flash_chip *, __u32, __u32, __u32 *, __u8 *);
	int (*write)(struct flash_chip *, __u32, __u32 , __u32 *, const __u8 *);
	int (*erase)(struct flash_chip *, struct erase_opt *);

	int (*read_oob)(struct flash_chip *, __u32, struct oob_opt *);
	int (*write_oob)(struct flash_chip *,__u32, struct oob_opt *);

	int (*block_is_bad)(struct flash_chip *, __u32);
	int (*block_mark_bad)(struct flash_chip *, __u32);
	int (*scan_bad_block)(struct flash_chip *); // fixme: to be removed

	OOB_MODE oob_mode;
};

static __u32 inline flash_write_is_align(struct flash_chip *flash, __u32 size)
{
	return (size + flash->write_size - 1) & ~(flash->write_size - 1);
}

static __u32 inline flash_erase_is_align(struct flash_chip *flash, __u32 size)
{
	return (size + flash->erase_size - 1) & ~(flash->erase_size - 1);
}

int flash_register(struct flash_chip *flash);

int flash_unregister(struct flash_chip *flash);

typedef enum {
	NAND_ECC_NONE,
	NAND_ECC_SW,
	NAND_ECC_HW,
//	NAND_ECC_YAFFS,
	NAND_ECC_YAFFS2,
} ECC_MODE;

int flash_set_ecc_mode(struct flash_chip *flash, ECC_MODE newMode, ECC_MODE *pOldMode);

int flash_fops_init(struct block_device *bdev);
