#pragma once

#include <block.h>

struct generic_drive
{
	struct block_device blk_dev;

	int (*get_block)(struct generic_drive *drive, int start, u8 buff[]);
	int (*put_block)(struct generic_drive *drive, int start, const u8 buff[]);

	union
	{
		size_t sect_size;
		struct generic_drive *master;
	};
};

int generic_drive_register(struct generic_drive *drive);
