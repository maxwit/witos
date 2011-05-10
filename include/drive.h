#pragma once

#include <block.h>

struct generic_drive
{
	char name[32]; // fixme

	struct block_device blk_dev;

	size_t drive_size;
	size_t block_size;

	int (*get_block)(struct generic_drive *drive, int start, u8 buff[]);
	int (*put_block)(struct generic_drive *drive, int start, const u8 buff[]);
};

int generic_drive_register(struct generic_drive *drive);
