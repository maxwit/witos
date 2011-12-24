#include <flash/flash.h>

static DECL_INIT_LIST(g_master_list);
static int g_flash_count = 0;

/*
 * g-bios flash partition definition
 * original from Linux kernel (driver/mtd/cmdlinepart.c)
 *
 * mtdparts  := <mtddef>[;<mtddef]
 * <mtddef>  := <mtd-id>:<partdef>[,<partdef>]
 *              where <mtd-id> is the name from the "cat /proc/mtd" command
 * <partdef> := <size>[@offset][<name>][ro][lk]
 * <mtd-id>  := unique name used in mapping driver/device (mtd->name)
 * <size>    := standard linux memsize or "-" to denote all remaining space
 * <name>    := '(' NAME ')'
 *
 * Examples:  1 NOR Flash with 2 partitions, 1 NAND with one
 * edb7312-nor:256k(ARMboot)ro,-(root);edb7312-nand:-(home)
 *
 *
 * fixme:
 * 1. to fix cross/overlapped parts
 * 2. as an API and called from flash core
 * 3. to support <partdef> := <size>[@offset][(<label>[, <image_name>, <image_size>])]
 * 4. handle exception
 *
 */

static int __INIT__ flash_parse_part(struct flash_chip *host,
						struct part_attr *part,	const char *part_def)
{
	int i, ret = -EINVAL, index = 0;
	__u32 curr_base = 0;
	char buff[128];
	const char *p;

	p = strchr(part_def, ':');
	if (!p)
		goto error;
	p++;

	while (*p && *p != ';') {
		if (curr_base >= host->chip_size)
			goto error;

		while (' ' == *p) p++;

		// part size
		if (*p == '-') {
			part->size = host->chip_size - curr_base;
			p++;
		} else {
			for (i = 0; *p && *p!= '@' && *p != '('; i++, p++)
				buff[i] = *p;
			buff[i] = '\0';

			ret = hr_str_to_val(buff, &part->size);
			if (ret < 0)
				goto error;

			ALIGN_UP(part->size, host->erase_size);
		}

		// part base
		if (*p == '@') {
			for (i = 0, p++; *p && *p != '('; i++, p++)
				buff[i] = *p;
			buff[i] = '\0';

			ret = hr_str_to_val(buff, &part->base);
			if (ret < 0)
				goto error;

			ALIGN_UP(part->base, host->erase_size);

			curr_base = part->base;
		} else {
			part->base = curr_base;
		}

		// part label and image
		i = 0;
		if (*p == '(') {
			for (p++; *p && *p != ')'; i++, p++)
				part->label[i] = *p;

			p++;
			if (',' == *p)
				p++;
		}
		part->label[i] = '\0';

		curr_base += part->size;

		index++;
		part++;
	}

	return index;

error:
	printf("%s(): invalid part definition \"%s\"\n", __func__, part_def);
	return ret;
}

static int __INIT__ flash_scan_part(struct flash_chip *host,
						struct part_attr part[])
{
	int ret;
	char part_def[CONF_VAL_LEN];

	ret = conf_get_attr("flash.part", part_def);
	if (ret < 0)
		return ret;

	// TODO: add code here!

	return flash_parse_part(host, part, part_def);
}

static int part_read(struct flash_chip *slave,
				__u32 from, __u32 len, __u32 *retlen, __u8 *buff)
{
	struct flash_chip *master = slave->master;

	return master->read(master, slave->bdev.base + from, len, retlen, buff);
}

static int part_write(struct flash_chip *slave,
				__u32 to, __u32 len, __u32 *retlen, const __u8 *buff)
{
	struct flash_chip *master = slave->master;

	return master->write(master, slave->bdev.base + to, len, retlen, buff);
}

static int part_erase(struct flash_chip *slave, struct erase_opt *opt)
{
	struct flash_chip *master = slave->master;

	opt->estart += slave->bdev.base;
	return master->erase(master, opt);
}

static int part_read_oob(struct flash_chip *slave,
				__u32 from, struct oob_opt *ops)
{
	struct flash_chip *master = slave->master;

	return master->read_oob(master, slave->bdev.base + from, ops);
}

static int part_write_oob(struct flash_chip *slave,
				__u32 to,	struct oob_opt *opt)
{
	struct flash_chip *master = slave->master;

	return master->write_oob(master, slave->bdev.base + to, opt);
}

static int part_block_is_bad(struct flash_chip *slave, __u32 off)
{
	struct flash_chip *master = slave->master;

	return master->block_is_bad(master, slave->bdev.base + off);
}

static int part_block_mark_bad(struct flash_chip *slave, __u32 off)
{
	struct flash_chip *master = slave->master;

	return master->block_mark_bad(master, slave->bdev.base + off);
}

int flash_register(struct flash_chip *flash)
{
	int i, n, ret;
	struct flash_chip *slave;
	struct part_attr part_tab[MAX_FLASH_PARTS];

	printf("registering flash device \"%s\":\n", flash->name);
	list_add_tail(&flash->master_node, &g_master_list);

	n = flash_scan_part(flash, part_tab);

	if (n <= 0) {
		snprintf(flash->bdev.name, MAX_DEV_NAME,
			BDEV_NAME_FLASH "%c", 'A' + g_flash_count);

		flash_fops_init(&flash->bdev); // fixme: not here!
		ret = block_device_register(&flash->bdev);
		// if ret < 0 ...
	} else {
		list_head_init(&flash->slave_list);

		for (i = 0; i < n; i++) {
			slave = zalloc(sizeof(*slave));
			if (NULL == slave)
				return -ENOMEM;

			snprintf(slave->bdev.name, PART_NAME_LEN, BDEV_NAME_FLASH "%d", i + 1);

			slave->bdev.base = part_tab[i].base;
			slave->bdev.size = part_tab[i].size;
			strncpy(slave->bdev.label, part_tab[i].label,
				sizeof(slave->bdev.label));

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

			slave->read  = part_read;
			slave->write = part_write;
			slave->erase = part_erase;
			slave->read_oob  = part_read_oob;
			slave->write_oob = part_write_oob;
			slave->block_is_bad   = part_block_is_bad;
			slave->block_mark_bad = part_block_mark_bad;
			slave->scan_bad_block = flash->scan_bad_block; // fixme

			list_add_tail(&slave->slave_node, &flash->slave_list);

			flash_fops_init(&slave->bdev);
			ret = block_device_register(&slave->bdev);
			// if ret < 0 ...
		}
	}

	g_flash_count++;

	return ret;
}

int flash_unregister (struct flash_chip *flash)
{
	// TODO: check master or not

	return 0;
}

static int __INIT__ flash_init(void)
{
	return 0;
}

SUBSYS_INIT(flash_init);
