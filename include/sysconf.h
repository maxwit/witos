#pragma once

#include <flash/part.h>
#include <net/net.h>

#define GB_SYSCFG_VER       5

// fixme for page > 2K
// #define FLASH_CONFIG_SIZE    KB(128)
// #define FLASH_CONFIG_ADDR    (MB(1) - FLASH_CONFIG_SIZE) // fixme


#define SC_RESET_MBR        (1 << 0)
#define SC_RESET_NET        (1 << 1)
#define SC_RESET_KERNEL     (1 << 2)
#define SC_RESET_ALL        (~0)


#define CON_DEV_NAME_LEN    63
#define DEFAULT_KCMDLINE_LEN     512

#define BM_MASK        ((1 << 8) - 1)

#define BM_RAMDISK     (1 << 8)
#define BM_FLASHDISK   (2 << 8)
#define BM_NFS         (4 << 8)
#define BM_CMDLINE     (8 << 8)


struct linux_config
{
	u32  boot_mode;

	int  root_dev;

	char kernel_image[MAX_FILE_NAME_LEN];
	char ramdisk_image[MAX_FILE_NAME_LEN];

	// char szNfsSvr[IPV4_STR_LEN];
	char nfs_path[NFS_PATH_LEN];

	char console_device[CON_DEV_NAME_LEN + 1];
	u32  mach_id;

	char cmd_line[DEFAULT_KCMDLINE_LEN];
};


int sysconf_get_part_info(struct part_info **ppPartInfo); // fixme: to be removed

int sysconf_get_net_info(struct net_config **ppNetConf);

int sysconf_get_linux_param(struct linux_config **ppLinuxParam);

int sysconf_reset();

int sysconf_save(void);

void sysconf_init_part_tab(const struct part_attr *attr, int n);

