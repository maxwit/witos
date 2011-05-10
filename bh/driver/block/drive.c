#include <stdio.h>
#include <block.h>
#include <drive.h>

#define MBR_PART_TAB_OFF 0x1BE
#define MSDOS_MAX_PARTS 16

struct msdos_part
{
	u8  boot_flag;
	u8  chs_start[3];
	u8  type;
	u8  chs_end[3];
	u32 lba_start;
	u32 lba_size;
};

static struct list_node g_master_list;

// fixme to support extened partion
static int msdos_part_scan(struct generic_drive *drive, struct part_attr part_tab[])
{
	int i;
	struct msdos_part dos_pt[MSDOS_MAX_PARTS];

	assert(drive != NULL);

	u8 buff[drive->blk_dev.sect_size];

	drive->get_block(drive, 0, buff);

	memcpy(dos_pt, buff + MBR_PART_TAB_OFF, sizeof(dos_pt));

	for (i = 0; i < 4; i++)
	{
		if (dos_pt[i].lba_size == 0)
			break;

		// fixme
		part_tab[i].part_type = PT_NONE;
		part_tab[i].part_name[0] = '\0';

		part_tab[i].part_base = dos_pt[i].lba_start * drive->blk_dev.sect_size;
		part_tab[i].part_size = dos_pt[i].lba_size * drive->blk_dev.sect_size;

		printf("0x%08x - 0x%08x (%dM)\n",
			dos_pt[i].lba_start, dos_pt[i].lba_start + dos_pt[i].lba_size,
			dos_pt[i].lba_size * drive->blk_dev.sect_size >> 20);
	}

	return i;
}

static int drive_get_block(struct generic_drive *drive, int start, void *buff)
{
	struct generic_drive *master = drive->master;

	return master->get_block(master, drive->blk_dev.bdev_base + start, buff);
}

static int drive_put_block(struct generic_drive *drive, int start, const void *buff)
{
	struct generic_drive *master = drive->master;

	return master->put_block(master, drive->blk_dev.bdev_base + start, buff);
}

int generic_drive_register(struct generic_drive *drive)
{
	int ret, num, i;
	struct part_attr part_tab[MSDOS_MAX_PARTS];
	struct generic_drive *slave;

	ret = block_device_register(&drive->blk_dev);
	// if ret < 0 ...

	num = msdos_part_scan(drive, part_tab);
	// if num < 0 ...

	for (i = 0; i < num; i++)
	{
		slave = zalloc(sizeof(*slave));
		// if ...

		snprintf(slave->blk_dev.dev.name, PART_NAME_LEN, "%sp%d", drive->blk_dev.dev.name, i);

		slave->blk_dev.bdev_base = part_tab[i].part_base;
		slave->blk_dev.bdev_size = part_tab[i].part_size;

		slave->master = drive;
		list_add_tail(&slave->slave_node, &drive->slave_list);

		slave->get_block = drive_get_block;
		slave->put_block = drive_put_block;

		ret = block_device_register(&slave->blk_dev);
		// if ret < 0 ...
	}

	list_add_tail(&drive->master_node, &g_master_list);

	return 0;
}

static int __INIT__ generic_drive_init(void)
{
	list_head_init(&g_master_list);
	return 0;
}

SUBSYS_INIT(generic_drive_init);
