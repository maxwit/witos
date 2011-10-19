#include <sysconf.h>
#include <flash/flash.h>

static struct list_node g_master_list;

static const struct part_attr *g_part_attr;
static int g_part_num = 0;

void __INIT__ flash_add_part_tab(const struct part_attr *attr, int num)
{
	g_part_attr = attr;
	g_part_num  = num;
}

// fixme:
// 1. to fix cross/overlapped parts
// 2. as an API and called from flash core
static int __INIT__ flash_adjust_part_tab(struct flash_chip *host,
						struct part_attr *new_attr,	const struct part_attr *attr_tmpl, u32 parts)
{
	int index = 0;
	u32 curr_base;
	struct linux_config *linux_conf = sysconf_get_linux_param();

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
				new_attr->part_base = CONFIG_GBH_START_BLK * host->erase_size;

				if (new_attr->part_base < curr_base)
				{
					printf("Invalid BH start block!\n");
					return -EINVAL;
				}
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
				new_attr->part_base != CONFIG_GBH_START_BLK * host->erase_size)
			{
				printf("%s() line %d: fixme!\n", __func__, __LINE__);
			}
			else if (new_attr->part_type >= PT_FS_BEGIN && new_attr->part_type <= PT_FS_END)
			{
				linux_conf->root_dev = index;
				break;
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

#if 0
		if (PT_BL_GCONF == new_attr->part_type)
		{
			host->conf_attr = new_attr;
		}
		else if (PT_OS_LINUX == new_attr->part_type)
		{
			u32 end, gap;

			end = new_attr->part_base + new_attr->part_size;
			gap = end & (MB(1) - 1);

			if (gap)
			{
				new_attr->part_size += MB(1) - gap;
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
			// strncpy(new_attr->part_name, part_type2str(new_attr->part_type), sizeof(new_attr->part_name));
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

// TODO: support tab/chip macthing

static int g_flash_count = 0;

static int part_read(struct flash_chip *slave,
				u32 from, u32 len, u32 *retlen, u8 *buff)
{
	struct flash_chip *master = slave->master;

	return master->read(master, slave->bdev.bdev_base + from, len, retlen, buff);
}

static int part_write(struct flash_chip *slave,
				u32 to, u32 len, u32 *retlen, const u8 *buff)
{
	struct flash_chip *master = slave->master;

	return master->write(master, slave->bdev.bdev_base + to, len, retlen, buff);
}

static int part_erase(struct flash_chip *slave, struct erase_opt *opt)
{
	struct flash_chip *master = slave->master;

	opt->estart += slave->bdev.bdev_base;
	return master->erase(master, opt);
}

static int part_read_oob(struct flash_chip *slave,
				u32 from, struct oob_opt *ops)
{
	struct flash_chip *master = slave->master;

	return master->read_oob(master, slave->bdev.bdev_base + from, ops);
}

static int part_write_oob(struct flash_chip *slave,
				u32 to,	struct oob_opt *opt)
{
	struct flash_chip *master = slave->master;

	return master->write_oob(master, slave->bdev.bdev_base + to, opt);
}

static int part_block_is_bad(struct flash_chip *slave, u32 off)
{
	struct flash_chip *master = slave->master;

	return master->block_is_bad(master, slave->bdev.bdev_base + off);
}

static int part_block_mark_bad(struct flash_chip *slave, u32 off)
{
	struct flash_chip *master = slave->master;

	return master->block_mark_bad(master, slave->bdev.bdev_base + off);
}

int flash_register(struct flash_chip *flash)
{
	int i, n, ret;
	struct flash_chip *slave;
	struct part_attr part_tab[MAX_FLASH_PARTS];

	snprintf(flash->bdev.dev.name, MAX_DEV_NAME, "mtdblock%d", g_flash_count);
	g_flash_count++;

	ret = block_device_register(&flash->bdev);
	// if ret < 0 ...
	list_head_init(&flash->slave_list);
	list_add_tail(&flash->master_node, &g_master_list);

	n = flash_adjust_part_tab(flash,
			part_tab, g_part_attr, g_part_num);

	for (i = 0; i < n; i++)
	{
		slave = zalloc(sizeof(*slave));
		if (NULL == slave)
			return -ENOMEM;

		// fixme
		snprintf(slave->bdev.dev.name, PART_NAME_LEN, "%sp%d",
			flash->bdev.dev.name, i + 1);

		slave->bdev.bdev_base = part_tab[i].part_base;
		slave->bdev.bdev_size = part_tab[i].part_size;

		slave->write_size  = flash->write_size;
		slave->erase_size  = flash->erase_size;
		slave->chip_size   = flash->chip_size;
		slave->write_shift = flash->write_shift;
		slave->erase_shift = flash->erase_shift;
		slave->chip_shift  = flash->chip_shift;
		slave->type        = flash->type;
		slave->oob_size    = flash->oob_size;
		slave->oob_mode    = flash->oob_mode;
		slave->master      = flash;
		list_add_tail(&slave->slave_node, &flash->slave_list);

		slave->read  = part_read;
		slave->write = part_write;
		slave->erase = part_erase;
		slave->read_oob  = part_read_oob;
		slave->write_oob = part_write_oob;
		slave->block_is_bad   = part_block_is_bad;
		slave->block_mark_bad = part_block_mark_bad;
		slave->scan_bad_block = flash->scan_bad_block; // fixme

		ret = block_device_register(&slave->bdev);
	}

#if 0
	part_show(flash);

	if (PT_BL_GBH == part->attr->part_type)
	{
		part_set_home(0);
		part_change(0);
	}
#endif

	return ret;
}

int flash_unregister (struct flash_chip *flash)
{
	// TODO: check master or not

	return 0;
}

static int __INIT__ flash_core_init(void)
{
	list_head_init(&g_master_list);

	return 0;
}

SUBSYS_INIT(flash_core_init);
