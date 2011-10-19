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

// Linux kernel and ramdisk
#define NFS_PATH_LEN   256

struct linux_config
{
	u32  boot_mode;

	int  root_dev;

	char kernel_image[MAX_FILE_NAME_LEN];
	char ramdisk_image[MAX_FILE_NAME_LEN];

	char nfs_path[NFS_PATH_LEN];

	char console_device[CONSOLE_DEV_NAME_LEN + 1];
	u32  mach_id;

	char cmd_line[DEFAULT_KCMDLINE_LEN];
};

// networking configuration
#define MAX_IFX_NUM    16

struct ifx_config
{
	char name[NET_NAME_LEN];
	u32  local_ip;
	u32  net_mask;
	u8   mac_addr[MAC_ADR_LEN];
};

struct net_config
{
	u32  server_ip;
	struct ifx_config net_ifx[MAX_IFX_NUM];
};

int net_get_server_ip(u32 *ip);

int net_set_server_ip(u32 ip);

struct image_info *sysconf_get_image_info(void);

struct net_config *sysconf_get_net_info(void);

struct linux_config *sysconf_get_linux_param(void);

int __INIT__ sysconf_init(void);

int sysconf_save(void);

/////////////////////////////
struct sys_config
{
	__u32 magic;
	__u32 size;
	__u32 offset;
	__u32 checksum;
};

// API list
int conf_load(void);
int conf_get_attr(const char *attr, char val[]);
int conf_set_attr(const char *attr, const char *val);
int conf_add_attr(const char *attr, const char *val);
int conf_del_attr(const char *attr);
int conf_store(void);
