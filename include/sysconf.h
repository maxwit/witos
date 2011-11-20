#pragma once

#include <fs/fs.h>
#include <net/net.h>

#define GB_SYSCFG_VER       7

#define GB_SYSCFG_MAGICNUM  0x5a5a5a5a

#define CONSOLE_DEV_NAME_LEN   63
#define DEFAULT_KCMDLINE_LEN   512

#define BM_MASK        ((1 << 8) - 1)

#define BM_RAMDISK     (1 << 8)
#define BM_FLASHDISK   (2 << 8)
#define BM_NFS         (4 << 8)
#define BM_MMC         (8 << 8)
#define BM_TFTP        (16 << 8)

int __INIT__ sysconf_init(void);

/////////////////////////////
struct sys_config
{
	__u32 magic;
	__u32 size;
	__u32 offset;
	__u32 checksum;
};

#define CONF_VAL_LEN 512
#define CONF_ATTR_LEN 128

// default net config attribute value
#define DEFAULT_NETMASK       "255.255.255.0"
#define DEFAULT_SERVER_IP     "10.0.0.1"
#define DEFAULT_LOCAL_IP      "10.0.0.2"
#define DEFAULT_NAME_IFX(n)   "eth"#n
#define DEFAULT_MAC_ADDR      "10:20:30:40:50:60"

// default linux config attribute value
#define DEFAULT_BOOT_MODE     0x1234
#define DEFAULT_KCMDLINE      "root=/dev/mtdblock3"
#define DEFAULT_CONSOLE_DEV   "ttyS0"
#define DEFAULT_KERNEL_IMG    "zImage"
#define DEFAULT_RAMDISK       "initrd"
#define DEFAULT_MACH_ID       0x5000
#define DEFAULT_NFS_PATH      "/maxwit/image/boot"
#define DEFAULT_ROOT_DEV      2

#ifdef CONFIG_DEBUG
#define DPRINT_ATTR(attr, dstr) \
							do {\
								printf("%s(), line %d\n", __func__, __LINE__);\
								printf("%s %s\n", attr, dstr);\
							} while (0)
#else

#define DPRINT_ATTR(attr, dstr) do {} while (0)

#endif

#define ATTR_NOT_FOUND "attribute is not found, it will be use default value!"

#define ATTR_FMT_ERR "string format error, it will be use default value!"

void set_load_mem_addr(__u32 *addr);
__u32 get_load_mem_addr();

// API list
int conf_load(void);
int conf_get_attr(const char *attr, char val[]);
int conf_set_attr(const char *attr, const char *val);
int conf_add_attr(const char *attr, const char *val);
int conf_del_attr(const char *attr);
int conf_list_attr(void);
int conf_store(void);
