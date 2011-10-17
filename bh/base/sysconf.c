#include <sysconf.h>
#include <net/net.h>
#include <flash/flash.h>

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

static struct sysconf_data *g_sysconf_data;

struct image_info *sysconf_get_image_info(void)
{
	return g_sysconf_data->image_conf;
}

struct net_config *sysconf_get_net_info(void)
{
	return &g_sysconf_data->net_conf;
}

struct linux_config *sysconf_get_linux_param(void)
{
	return &g_sysconf_data->linux_conf;
}

static u32 sysconf_checksum(u32 *new_sum)
{
	u32 old_sum = g_sysconf_data->checksum;

	g_sysconf_data->checksum = 0;
	g_sysconf_data->checksum = ~net_calc_checksum(&g_sysconf_data, sizeof(g_sysconf_data));
	g_sysconf_data->checksum |= GB_SYSCFG_VER << 16;

	if (NULL != new_sum)
		*new_sum = g_sysconf_data->checksum;

	return old_sum;
}

static int __INIT__ sysconf_load(void)
{
	u32 old_sum, new_sum;

	g_sysconf_data = (struct sysconf_data *)(CONFIG_SYS_START_MEM + 36);

	old_sum = sysconf_checksum(&new_sum);
	if (old_sum != new_sum)
	{
		printf("checksum error! (0x%08x != 0x%08x)\n", new_sum, old_sum);
		return -EINVAL;
	}

	return 0;
}

int sysconf_save(void)
{
	int ret;
	u32 conf_base, conf_size;
	u8 *conf_buff;
	struct flash_chip *flash;
	struct sysconf_data *sysconf = g_sysconf_data;

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		return -ENODEV;
	}

	conf_base = flash->erase_size * CONFIG_SYS_START_BLK;
	conf_size = *(__u32 *)(CONFIG_SYS_START_MEM + GBH_SIZE_OFFSET); // fixme

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

int __INIT__ sysconf_init(void)
{
	int ret;

	ret = sysconf_load();
	// ...

	return ret;
}
