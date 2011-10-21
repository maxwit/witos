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

const char *part_type2str(__u32 type);
