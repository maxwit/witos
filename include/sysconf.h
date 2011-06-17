#pragma once

#include <flash/part.h>
#include <net/net.h>

#define GB_SYSCFG_VER       7

// fixme for page > 2K
// #define FLASH_CONFIG_SIZE    KB(128)
// #define FLASH_CONFIG_ADDR    (MB(1) - FLASH_CONFIG_SIZE) // fixme

#define SC_RESET_MBR        (1 << 0)
#define SC_RESET_NET        (1 << 1)
#define SC_RESET_KERNEL     (1 << 2)
#define SC_RESET_ALL        (~0)

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

int sysconf_reset(void);

int sysconf_activate(void);

int sysconf_save(void);
