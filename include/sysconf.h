#pragma once

#include <types.h>

#define GB_SYSCFG_VER    7
#define GB_SYSCFG_MAGIC  MKFOURCC('G', 's', 'y', 's')

// End of the file
#define EOF 0xFF

#define CONSOLE_DEV_NAME_LEN   63
#define DEFAULT_KCMDLINE_LEN   512

#define BM_MASK        ((1 << 8) - 1)

#define BM_RAMDISK     (1 << 8)
#define BM_FLASHDISK   (2 << 8)
#define BM_NFS         (4 << 8)
#define BM_MMC         (8 << 8)
#define BM_TFTP        (16 << 8)

/////////////////////////////

#define CONF_VAL_LEN 512
#define CONF_ATTR_LEN 128

// default net config attribute value
#define DEFAULT_NAME_IFX(n)   "eth"#n

// default linux config attribute value
#define DEFAULT_BOOT_MODE     0x1234
#define DEFAULT_KCMDLINE      "root=/dev/mtdblock3"
#define DEFAULT_CONSOLE_DEV   "ttyS0"
#define DEFAULT_KERNEL_IMG    "zImage"
#define DEFAULT_RAMDISK       "initrd"
#define DEFAULT_MACH_ID       0x5000
#define DEFAULT_NFS_PATH      "/maxwit/image/boot"
#define DEFAULT_ROOT_DEV      2

void set_load_mem_addr(__u32 *addr);
__u32 get_load_mem_addr();

// API list
int conf_load(void);
void conf_reset(void);
int conf_get_attr(const char *attr, char val[]);
int conf_set_attr(const char *attr, const char *val);
int conf_add_attr(const char *attr, const char *val);
int conf_del_attr(const char *attr);
int conf_list_attr(void);
int conf_store(void);
