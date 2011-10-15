#include <flash/flash.h>
#include <flash/part.h>

static struct flash_chip *g_flash_list[MAX_FLASH_DEVICES];

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

		if (PT_BL_GCONF == new_attr->part_type)
		{
			host->conf_attr = new_attr;
		}
#if 0
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

static int __INIT__ attach_part_info(struct flash_chip *flash, struct part_info *pt_info)
{
	struct partition *part;
	struct part_attr *attr;

	flash->pt_info = pt_info;

	part  = flash->part_tab;
	attr  = pt_info->attr_tab;

	while (part < flash->part_tab + MAX_FLASH_PARTS)
	{
		part->attr  = attr;
		part->host  = flash;

		part++;
		attr++;
	}

	return 0;
}

static struct part_info g_part_info;

// TODO: support tab/chip macthing
int flash_register(struct flash_chip *flash)
{
	int i, ret = 0; // ret has no use currently

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
	{
		if (NULL == g_flash_list[i])
		{
			g_flash_list[i] = flash;

			if (g_part_num > 0)
			{
				g_part_info.parts = flash_adjust_part_tab(flash,
										g_part_info.attr_tab, g_part_attr, g_part_num);

				ret = attach_part_info(flash, &g_part_info);

				part_show(flash);
			}

			return i;
		}
	}

	return -EBUSY;
}

int flash_unregister (struct flash_chip *flash)
{
	int i;

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
	{
		if (flash == g_flash_list[i])
		{
			g_flash_list[i] = NULL;

			return i;
		}
	}

	return -ENODEV;
}

struct flash_chip *flash_get(unsigned int num)
{
	struct flash_chip *flash;

	if (num >= MAX_FLASH_DEVICES)
		return NULL;

	flash = g_flash_list[num];

	return flash;
}

struct flash_chip *flash_get_by_name(const char *name)
{
	int i;
	struct flash_chip *flash = NULL;

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
	{
		if (g_flash_list[i] && !strcmp(name, g_flash_list[i]->name))
		{
			flash = g_flash_list[i];
			break;
		}
	}

	return flash;
}

static int __INIT__ flash_core_init(void)
{
	int i;

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
		g_flash_list[i] = NULL;

	return 0;
}

SUBSYS_INIT(flash_core_init);
