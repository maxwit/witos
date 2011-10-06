#include <sysconf.h>
#include <net/net.h>
#include <flash/flash.h>
#include <flash/part.h>

struct sysconf_data
{
	u32 checksum;

	struct image_info   image_conf[MAX_FLASH_PARTS];
	struct net_config   net_conf;
	struct linux_config linux_conf;
};

#if 0
static const struct sysconf_data g_default_sysconf = {
	.net_conf = {
		.server_ip = CONFIG_SERVER_IP,
		.net_ifx = {
			{
				.name     = "eth0",
				.local_ip = CONFIG_LOCAL_IP,
				.net_mask = CONFIG_NET_MASK,
				.mac_addr = CONFIG_MAC_ADDR,
			},
			{}
		}
	}
};
#endif

static struct sysconf_data g_sysconf_data;

struct image_info *sysconf_get_image_info(void)
{
	return g_sysconf_data.image_conf;
}

struct net_config *sysconf_get_net_info(void)
{
	return &g_sysconf_data.net_conf;
}

struct linux_config *sysconf_get_linux_param(void)
{
	return &g_sysconf_data.linux_conf;
}

static u32 sysconf_checksum(u32 *new_sum)
{
	u32 old_sum = g_sysconf_data.checksum;

	g_sysconf_data.checksum = 0;
	g_sysconf_data.checksum = ~net_calc_checksum(&g_sysconf_data, sizeof(g_sysconf_data));
	g_sysconf_data.checksum |= GB_SYSCFG_VER << 16;

	if (NULL != new_sum)
		*new_sum = g_sysconf_data.checksum;

	return old_sum;
}

int sysconf_reset(void)
{
	int i;
	u8 mac[] = CONFIG_MAC_ADDR;
	struct image_info *image_conf = sysconf_get_image_info();
	struct net_config *net_conf = sysconf_get_net_info();
	struct linux_config *linux_conf = sysconf_get_linux_param();
	struct flash_chip *flash;

	memset(net_conf, 0x0, sizeof(*net_conf));
	memset(linux_conf, 0x0, sizeof(*linux_conf));

	// network reset
	net_conf->server_ip = CONFIG_SERVER_IP;
	// fixme!!!
	strncpy(net_conf->net_ifx[0].name, "eth0", NET_NAME_LEN);
	net_conf->net_ifx[0].local_ip = CONFIG_LOCAL_IP;
	net_conf->net_ifx[0].net_mask = CONFIG_NET_MASK;
	memcpy(net_conf->net_ifx[0].mac_addr, mac, MAC_ADR_LEN);

	// linux reset
	// fixme!!!
	linux_conf->kernel_image[0] = '\0';
	linux_conf->ramdisk_image[0] = '\0'; // to be removed
	linux_conf->boot_mode = BM_FLASHDISK;

	// fixme
	linux_conf->root_dev = MAX_FLASH_PARTS;

	flash = flash_open(BOOT_FLASH_ID);
	if (flash)
	{
		struct part_info *pt_info = flash->pt_info;

		for (i = 0; i < pt_info->parts; i++)
		{
			const struct part_attr *attr = pt_info->attr_tab + i;

			if (attr->part_type >= PT_FS_BEGIN && attr->part_type <= PT_FS_END)
			{
				linux_conf->root_dev = i;
				break;
			}
		}

		if (MAX_FLASH_PARTS == linux_conf->root_dev)
		{
			printf("Warning: root device not defined!\n");
			linux_conf->root_dev = 0;
		}

		flash_close(flash);
	}

	strncpy(linux_conf->nfs_path, CONFIG_NFS_ROOT, sizeof(linux_conf->nfs_path));

	// fixme: uart as console?
	snprintf(linux_conf->console_device, CONSOLE_DEV_NAME_LEN,
			CONFIG_CONSOLE_NAME "%d,115200", CONFIG_UART_INDEX);

	linux_conf->mach_id = CONFIG_MACH_TYPE;

	// image reset
	memset(image_conf, 0x0, sizeof(*image_conf) * MAX_FLASH_PARTS);

	// generate checksum
	sysconf_checksum(NULL);

	//fixme
	return 0;
}

static int __INIT__ sysconf_load(void)
{
	int ret = 0;
	u32 conf_size, conf_base;
	u32 old_sum, new_sum;
	u8 *conf_buff;
	struct flash_chip *flash;
	struct part_attr  *attr;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		return -ENODEV;
	}

	attr = flash->conf_attr;
	if (NULL == attr)
	{
		DPRINT("%s(), line %d\n", __func__, __LINE__);
		return -ENODEV;
	}

	conf_base = attr->part_base;
	conf_size = attr->part_size; // fixme
	// flash->erase_size > sizeof(struct sysconf_data) ? flash->erase_size : sizeof(struct sysconf_data);

	conf_buff = (u8 *)malloc(conf_size);
	if (NULL == conf_buff)
	{
		ret = -ENOMEM;
		goto L2;
	}

	DPRINT("%s(): base = 0x%08x, size = 0x%08x\n",
		__func__, conf_base, conf_size);

	ret = flash_read(flash, conf_buff, conf_base, conf_size);
	if (ret < 0)
	{
		goto L3;
	}

	memcpy(&g_sysconf_data, conf_buff, sizeof(struct sysconf_data));

	old_sum = sysconf_checksum(&new_sum);

	if (old_sum != new_sum)
	{
		printf("checksum error! (0x%08x != 0x%08x)\n", new_sum, old_sum);
		ret = -EIO;
	}

L3:
	free(conf_buff);
L2:
	flash_close(flash);

	return ret;
}

int sysconf_save(void)
{
	int ret;
	u32 conf_base, conf_size;
	u8 *conf_buff;
	struct flash_chip *flash;
	struct part_attr *attr;
	struct sysconf_data *sysconf = &g_sysconf_data;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		return -ENODEV;
	}

	attr = flash->conf_attr;
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

	sysconf_checksum(NULL);

	conf_buff = (u8 *)malloc(conf_size);
	if (NULL == conf_buff)
	{
		ret = -ENOMEM;
		goto L1;
	}

	memcpy(conf_buff, sysconf, sizeof(struct sysconf_data));

	ret = flash_write(flash, conf_buff, conf_size /* fixme */, conf_base);

	free(conf_buff);

L1:
	flash_close(flash);

	return ret;
}

int net_get_server_ip(u32 *ip)
{
	struct net_config *net_cfg;

	net_cfg = sysconf_get_net_info();

	if (NULL == net_cfg)
	{
		BUG();
		return -EIO;
	}

	*ip = net_cfg->server_ip;

	return 0;
}

int net_set_server_ip(u32 ip)
{
	struct net_config *net_cfg;

	net_cfg = sysconf_get_net_info();

	if (NULL == net_cfg)
	{
		BUG();
		return -EIO;
	}

	net_cfg->server_ip = ip;

	return 0;
}

int sysconf_activate(void)
{
	int idx;
	struct image_info *image;
	struct net_config *net_conf;
	struct net_device *ndev;
	struct ifx_config *net_ifx;
	struct flash_chip *flash;
	struct partition  *part;

	net_conf = sysconf_get_net_info();
	net_ifx = net_conf->net_ifx;

	idx = 0;

	while (net_ifx->name[0] && idx < MAX_IFX_NUM)
	{
		ndev = net_get_dev(net_ifx->name);
		if (ndev)
		{
			int ret;

			ret = ndev_ioctl(ndev, NIOC_SET_IP, (void *)net_ifx->local_ip);
			//
			ret = ndev_ioctl(ndev, NIOC_SET_MASK, (void *)net_ifx->net_mask);
			//
			ret = ndev_ioctl(ndev, NIOC_SET_MAC, net_ifx->mac_addr);
			//
		}

		net_ifx++;
		idx++;
	}

	flash = flash_open(BOOT_FLASH_ID);
	if (!flash)
	{
		DPRINT("%s() line %d: fail to open flash[%d]\n",
			__func__, __LINE__, BOOT_FLASH_ID);

		return -ENODEV;
	}

	part  = flash->part_tab;
	image = sysconf_get_image_info();

	for (idx = 0; idx < flash->pt_info->parts; idx++)
	{
		part->image = image;

		if (PT_BL_GBH == part->attr->part_type)
		{
			part_set_home(idx);
			part_change(idx);
		}

		part++;
		image++;
	}

	flash_close(flash);

	// ..
	return 0;
}

static int __INIT__ sysconf_init(void)
{
	int ret;

	ret = sysconf_load();

	if (ret < 0)
	{
		printf("%s() failed (errno = %d), setting sysconf to default!\n",
			__func__, ret);

		sysconf_reset();

		if (ret == -EIO)
		{
			ret = sysconf_save();

			if (ret < 0)
			{
				printf("sysconf_save() failed (errno = %d)!\n", ret);
			}
		}
	}

	ret = sysconf_activate();

	return ret;
}

APP_INIT(sysconf_init);
