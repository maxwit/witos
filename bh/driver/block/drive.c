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

// fixme to support extened partion
static int msdos_part_scan(struct generic_drive *drive, struct part_attr part_tab[])
{
	int i;
	struct msdos_part dos_pt[MSDOS_MAX_PARTS];

	assert(drive != NULL);

	u8 buff[drive->block_size];

	drive->get_block(drive, 0, buff);

	memcpy(dos_pt, buff + MBR_PART_TAB_OFF, sizeof(dos_pt));

	for (i = 0; i < 4; i++)
	{
		if (dos_pt[i].lba_size == 0)
			break;

		// fixme
		part_tab[i].part_type = PT_NONE;
		part_tab[i].part_name[0] = '\0';

		part_tab[i].part_base = dos_pt[i].lba_start * drive->block_size;
		part_tab[i].part_size = dos_pt[i].lba_size * drive->block_size;

		printf("0x%08x - 0x%08x (%dM)\n",
			dos_pt[i].lba_start, dos_pt[i].lba_start + dos_pt[i].lba_size,
			dos_pt[i].lba_size * drive->block_size >> 20);
	}

	return i;
}

int generic_drive_register(struct generic_drive *drive)
{
	int ret, num, i;
	struct part_attr part_tab[MSDOS_MAX_PARTS];
	struct generic_drive *slave;
	struct block_device *blk_dev;

	blk_dev = &drive->blk_dev;

	blk_dev->part_base = 0;
	blk_dev->part_size = drive->drive_size;
	strncpy(blk_dev->part_name, drive->name, PART_NAME_LEN);

	ret = block_device_register(&drive->blk_dev);
	// if ret < 0 ...

	num = msdos_part_scan(drive, part_tab);
	// if num < 0 ...

	for (i = 0; i < num; i++)
	{
		slave = zalloc(sizeof(*slave));
		// if ...
		blk_dev = &slave->blk_dev;

		blk_dev->part_base = part_tab[i].part_base;
		blk_dev->part_size = part_tab[i].part_size;
		snprintf(blk_dev->part_name, PART_NAME_LEN, "%sp%d", drive->name, i);

		ret = block_device_register(blk_dev);
		// if ret < 0 ...
	}

	return 0;
}
