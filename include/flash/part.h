#pragma once

#include <flash/flash.h>


#define PT_STR_GB_TH    "g-bios-th"
#define PT_STR_GB_BH    "g-bios-bh"
#define PT_STR_GB_CONF  "sysconfig"
#define PT_STR_LINUX    "linux"
#define PT_STR_WINCE    "wince"
#define PT_STR_RAMDISK  "ramdisk"
#define PT_STR_CRAMFS   "cramfs"
#define PT_STR_JFFS2    "jffs2"
#define PT_STR_YAFFS    "yaffs"
#define PT_STR_YAFFS2   "yaffs2"
#define PT_STR_UBIFS    "ubifs"
#define PT_STR_FREE     "free"

#define PT_MAX (PT_FREE + 1)


struct part_info
{
	int parts;
	struct part_attr attr_tab[MAX_FLASH_PARTS];
};


//////////////////
typedef enum
{
	OP_RDONLY,
	OP_WRONLY,
	OP_RDWR,
} OP_MODE;


#define PART_CURR  0x100000
struct partition *part_open(int index, OP_MODE uMode);

int part_close(struct partition *part);

long part_read(struct partition *, void *, u32);

long part_write(struct partition *, const void *, u32);

int part_change(int index);

int part_tab_read(const struct flash_chip *host, struct part_attr pPartAttr[], int nPartNum);

const char *part_type2str(u32 type);

const char *part_get_name(const struct part_attr *part);

int part_get_index(const struct partition *part);

static int __INLINE__ part_get_attr(const struct partition *part, struct part_attr *attr)
{
	*attr = *part->attr;
	return 0;
}

static void __INLINE__ part_set_attr(const struct partition *part, struct part_attr *attr)
{
	*part->attr = *attr;
}

static int __INLINE__ part_get_type(const struct partition *part)
{
	BUG_ON(NULL == part || NULL == part->attr)
	return part->attr->part_type;
}

static int __INLINE__ part_get_base(struct partition *part)
{
	return part->attr->part_base;
}

static int __INLINE__ part_get_size(struct partition *part)
{
	return part->attr->part_size;
}

int part_get_image(const struct partition *part, char name[], u32 *size);

int part_set_image(struct partition *part, char name[], u32 size);

int part_get_home(void);

void part_set_home(int index);

int part_show(const struct flash_chip *flash);

