#include <g-bios.h>
#include <sysconf.h>
#include <net/net.h>
#include <flash/flash.h>
#include <flash/part.h>

// fixme
#include <arm/board_id.h>

#ifndef CONFIG_MACH_ID
#if defined(CONFIG_AT91SAM9261)
#define CONFIG_MACH_ID MACH_TYPE_AT91SAM9261EK
#elif defined(CONFIG_AT91SAM9263)
#define CONFIG_MACH_ID MACH_TYPE_AT91SAM9263EK
#elif defined(CONFIG_S3C2410)
#define CONFIG_MACH_ID MACH_TYPE_SMDK2410
#elif defined(CONFIG_S3C2440)
#define CONFIG_MACH_ID MACH_TYPE_MW2440
#elif defined(CONFIG_S3C6410)
#define CONFIG_MACH_ID MACH_TYPE_SMDK6410
#elif defined(CONFIG_PXA168)
#define CONFIG_MACH_ID MACH_TYPE_ASPENITE
#else
#warning "MACH ID is not defined!"
#define CONFIG_MACH_ID 0xFFFFFFFF
#endif
#endif


struct sys_config
{
	u32 checksum;

	struct net_config    net_conf;
	struct linux_config  linux_conf;
	struct part_info part_conf;		//4TODO: remove me!
};


static struct image_cache g_kernel_cache;
static struct image_cache g_ramdisk_cache;

static struct sys_config g_sys_conf;
//static struct part_info gPartInfo; // TODO:  remove me!
static struct part_info *g_part_info;

static const u8 g_def_mac[MAC_ADR_LEN] = CONFIG_MAC_ADDR;

static const struct part_attr *g_part_tab_tmpl;
static int g_part_tab_num = 0;


void sysconf_init_part_tab(const struct part_attr *attr, int n)
{
	g_part_tab_tmpl = attr;
	g_part_tab_num  = n;
}


struct image_cache *image_cache_get(PART_TYPE type)
{
	switch (type)
	{
	case PT_OS_LINUX:
		return &g_kernel_cache;

	case PT_FS_RAMDISK:
		return &g_ramdisk_cache;

	default:
		break;
	}

	return NULL;
}


// fixme: to fix cross/overlapped parts
static int __INIT__ adjust_part_table(struct flash_chip *host,
						struct part_attr *new_attr,	const struct part_attr *attr_tmpl, u32 parts)
{
	int index = 0;
	u32 curr_base;


	if (parts > MAX_FLASH_PARTS)
	{
		parts = MAX_FLASH_PARTS;
	}

	curr_base  = attr_tmpl[0].part_base;
	curr_base -= curr_base % host->erase_size; // fixme: to use macro

	while (index < parts)
	{
		if (curr_base + host->erase_size > host->chip_size)
			break;

		new_attr->part_type = attr_tmpl->part_type;

		if (0 == attr_tmpl->part_base)
		{
			if (PT_BL_GBH == new_attr->part_type)
			{
				new_attr->part_base = GBH_START_BLK * host->erase_size;

				if (new_attr->part_base < curr_base)
				{
					printf("Invalid BH start block!\n");
					return -EINVAL;
				}

				part_set_home(index);
			}
			else
			{
				new_attr->part_base = curr_base;
			}
		}
		else
		{
			// fixme
			if (PT_BL_GBH == new_attr->part_type && \
				new_attr->part_base != GBH_START_BLK * host->erase_size)
			{
				printf("%s() line %d: fixme!\n", __FUNCTION__, __LINE__);
			}

			new_attr->part_base = attr_tmpl->part_base;

			ALIGN_UP(new_attr->part_base, host->erase_size);

			curr_base = new_attr->part_base;
		}

		if (0 == attr_tmpl->part_size)
		{
			new_attr->part_size = host->chip_size - curr_base;
		}
		else
		{
			new_attr->part_size = attr_tmpl->part_size;

			ALIGN_UP(new_attr->part_size, host->erase_size);
		}

		if (PT_BL_GB_CONF == new_attr->part_type)
		{
			host->attr = new_attr;
		}
#if 0
		else if (PT_OS_LINUX == new_attr->part_type)
		{
			u32 ulEnd, ulGap;

			ulEnd = new_attr->part_base + new_attr->part_size;
			ulGap = ulEnd & (MB(1) - 1);

			if (ulGap)
			{
				new_attr->part_size += MB(1) - ulGap;
			}
		}
#endif

		if (new_attr->part_base + new_attr->part_size > host->chip_size)
		{
			break;
		}

		curr_base += new_attr->part_size;

		if ('\0' == attr_tmpl->part_name[0])
		{
			strncpy(new_attr->part_name, part_type2str(new_attr->part_type), sizeof(new_attr->part_name));
		}
		else
		{
			strncpy(new_attr->part_name, attr_tmpl->part_name, sizeof(new_attr->part_name));
		}

		new_attr++;
		attr_tmpl++;

		index++;
	}

#if 1
	if (curr_base + host->erase_size <= host->chip_size)
	{
		new_attr->part_type  = PT_FREE;
		new_attr->part_base = curr_base;
		new_attr->part_size = host->chip_size - curr_base;

		strncpy(new_attr->part_name, "free", sizeof(new_attr->part_name));

		index++;
	}
#endif

	return index;
}


static u32 sysconf_check_sum(u32 *pNewSum)
{
	u32 dwOldSum = g_sys_conf.checksum;

	g_sys_conf.checksum = 0;
	g_sys_conf.checksum = ~ net_calc_checksum(&g_sys_conf, sizeof(g_sys_conf) - sizeof(struct part_info)) << 16;
	g_sys_conf.checksum |=GB_SYSCFG_VER;

	if (NULL != pNewSum)
		*pNewSum = g_sys_conf.checksum;

	return dwOldSum;
}


int sysconf_reset()
{
	int i;
	struct net_config   *net_conf = &g_sys_conf.net_conf;
	struct linux_config *linux_conf = &g_sys_conf.linux_conf;


	memset(&g_sys_conf, 0x0, sizeof(g_sys_conf) - sizeof(struct part_info)); // to be removed

	net_conf->local_ip = CONFIG_LOCAL_IP;
	net_conf->net_mask = CONFIG_NET_MASK;

	net_conf->server_ip = CONFIG_SERVER_IP;

	memcpy(net_conf->mac_adr, g_def_mac, MAC_ADR_LEN);
	net_set_mac(NULL, net_conf->mac_adr);

	// fixme!!!
	linux_conf->kernel_image[0] = '\0';

	linux_conf->ramdisk_image[0] = '\0'; // to be removed

	linux_conf->boot_mode = BM_FLASHDISK;

	linux_conf->root_dev = 0;

	for (i = 0; i < g_part_info->parts; i++)
	{
		const struct part_attr *attr = g_part_info->attr_tab + i;

		if (attr->part_type >= PT_FS_BEGIN && attr->part_type <= PT_FS_END)
		{
			linux_conf->root_dev = i;
			break;
		}
	}

	net_conf->server_ip = CONFIG_SERVER_IP;
	strncpy(linux_conf->nfs_path, CONFIG_NFS_ROOT, sizeof(linux_conf->nfs_path));

	snprintf(linux_conf->console_device, sizeof(linux_conf->console_device),
			"%s,115200", CONFIG_CONSOLE_NAME);

	linux_conf->mach_id = CONFIG_MACH_ID;

	sysconf_check_sum(NULL);

	//fixme
	return 0;
}


static int __INIT__ load_sysconf(void)
{
	int ret = 0, i;
	u32 conf_size, conf_base;
	u32 old_check_sum, new_check_sum;
	u8 *conf_buff;
	struct flash_chip     *flash;
	struct part_attr  *attr;
	struct part_info  *part;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		return -ENODEV;
	}

	attr = flash->attr;
	if (NULL == attr)
	{
		DPRINT("%s(), line %d\n", __FUNCTION__, __LINE__);
		return -ENODEV;
	}

	conf_base = attr->part_base;
	conf_size = attr->part_size;
	// flash->erase_size > sizeof(struct sys_config) ? flash->erase_size : sizeof(struct sys_config);

	conf_buff = (u8 *)malloc(conf_size);
	if (NULL == conf_buff)
	{
		ret = -ENOMEM;
		goto L2;
	}

	printf("%s(): base = 0x%08x, size = 0x%08x\n",
		__FUNCTION__, conf_base, conf_size);

	ret = flash_read(flash, conf_buff, conf_base, conf_size);
	if (ret < 0)
	{
		goto L3;
	}

	memcpy(&g_sys_conf, conf_buff, sizeof(struct sys_config) - sizeof(struct part_info));

	part = (struct part_info *)(conf_buff + sizeof(struct sys_config) - sizeof(struct part_info));
	for(i = 0; i < MAX_FLASH_PARTS; i++)
	{
		strncpy(g_sys_conf.part_conf.attr_tab[i].image_name, part->attr_tab[i].image_name, MAX_FILE_NAME_LEN);
		// fixme
		g_sys_conf.part_conf.attr_tab[i].image_name[MAX_FILE_NAME_LEN - 1] = 0;
		g_sys_conf.part_conf.attr_tab[i].image_size = part->attr_tab[i].image_size;
	}

	old_check_sum = sysconf_check_sum(&new_check_sum);

	if (old_check_sum != new_check_sum)
	{
		printf("checksum error! current = 0x%08x, original = 0x%08x.\n",
			new_check_sum, old_check_sum);

		ret = -EIO;
	}

L3:
	free(conf_buff);
L2:
	flash_close(flash);

	return ret;
}


// fixme!!
int sysconf_get_part_info(struct part_info **ppPartInfo)
{
	//*ppPartInfo = &gPartInfo;
	*ppPartInfo = g_part_info;

	return 0;
}


int sysconf_get_net_info(struct net_config **ppNetConf)
{
	*ppNetConf = &g_sys_conf.net_conf;

	return 0;
}


int sysconf_get_linux_param(struct linux_config **ppLinuxParam)
{
	*ppLinuxParam = &g_sys_conf.linux_conf;

	return 0;
}


int sysconf_save(void)
{
	int ret;
	u32 conf_base, conf_size;
	u8 *conf_buff;
    struct flash_chip *flash;
	struct part_attr *attr;
	struct sys_config *pSysConf = &g_sys_conf;


	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		return -ENODEV;
	}

	attr = flash->attr;
	if (NULL == attr)
	{
		return -ENODEV;
	}

	conf_base = attr->part_base;
	conf_size = attr->part_size;

	ret = flash_erase(flash, conf_base, conf_size, EDF_ALLOWBB);

	if (ret < 0)
	{
		goto L1;
	}

	sysconf_check_sum(NULL);

    conf_buff = (u8 *)malloc(conf_size);
    if (NULL == conf_buff)
	{
		ret = -ENOMEM;
		goto L1;
	}

    memcpy(conf_buff, pSysConf, sizeof(struct sys_config));

	ret = flash_write(flash, conf_buff, conf_size /* fixme */, conf_base);

    free(conf_buff);

L1:
	flash_close(flash);

	return ret;
}


static int __INIT__ attach_part_info(struct flash_chip *flash, struct part_info *pt_info)
{
	struct partition *part;
	struct part_attr  *attr;


	flash->pt_info = pt_info;

	part = flash->part_tab;
	attr = pt_info->attr_tab;


	while (part < flash->part_tab + MAX_FLASH_PARTS)
	{
		part->attr = attr;
		part->host = flash;

		part->img_cache = NULL;

		// fixme: move to part_open()
		switch (attr->part_type)
		{
		case PT_OS_LINUX:
			attr->buff_size = flash->erase_size;
			part->img_cache = &g_kernel_cache;
			part->img_cache->cache_base = malloc(attr->part_size); // fixme
			if (NULL == part->img_cache->cache_base)
			{
				printf("partition size of \"%s\" seems too large! (0x%08x)\n",
					part_type2str(attr->part_type), attr->part_size);
			}
			part->img_cache->cache_size = 0;
			break;

		case PT_FS_RAMDISK:
			part->img_cache = &g_ramdisk_cache;
			part->img_cache->cache_base = malloc(attr->part_size);
			if (NULL == part->img_cache->cache_base)
			{
				printf("partition size of \"%s\" seems too large! (0x%08x)\n",
					part_type2str(attr->part_type), attr->part_size);
			}
			part->img_cache->cache_size = 0;

		case PT_FS_CRAMFS:
		case PT_FS_JFFS2:
			attr->buff_size = flash->erase_size;
			break;

		case PT_FS_YAFFS:
		case PT_FS_YAFFS2:
			attr->buff_size = (flash->write_size + flash->oob_size) << (flash->erase_shift - flash->write_shift);
			break;

		default:
			break;
		}

		part++;
		attr++;
	}

	return 0;
}


static int __INIT__ flash_part_init(void)
{
	int ret;
	struct flash_chip *flash;

	g_part_info = &g_sys_conf.part_conf;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		ret = -ENODEV;
		goto L1;
	}

	if (g_part_tab_num <= 0)
	{
		printf("partition table not initialized!\n");
		ret = -EINVAL;
		goto L2;
	}

	g_part_info->parts = adjust_part_table(flash,
							g_part_info->attr_tab,
							g_part_tab_tmpl,
							g_part_tab_num
							);

	ret = attach_part_info(flash, g_part_info);

L2:
	flash_close(flash);
L1:
	return ret;
}



// TODO: add checking code
static int __INIT__ activate_sysconf(void)
{
	int ret;
	struct net_config *net_conf;

	ret = sysconf_get_net_info(&net_conf);

	if (!ret)
	{
		ret = net_set_mac(NULL, net_conf->mac_adr);
	}

	part_change(part_get_home());

	return ret;
}


static int __INIT__ sysconf_init(void)
{
	int ret;


	// create partition table for "boot" flash
	ret = flash_part_init();

	switch (ret)
	{
	case 0:
		// load system configuration from the specified config parition
		ret = load_sysconf();

		if (ret < 0)
		{
			printf("setting system configuration to default!\n");

			sysconf_reset();

			ret = sysconf_save();
			if (ret < 0)
			{
				printf("fail to store system configuration! (errno = %d)\n", ret);
			}
		}
		break;

	case -ENODEV:
		printf("no boot flash found, setting to default!\n");
		sysconf_reset();
		break;

	default:
		printf("fail to init flash partition! (errno = %d)\n", ret);
		goto L1;
	}

	// apply the config info for some devices
	ret = activate_sysconf();
L1:
	return ret;
}

APP_INIT(sysconf_init);

