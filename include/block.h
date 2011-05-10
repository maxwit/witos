#pragma once

struct generic_drive
{
	size_t drive_size;
	size_t block_size;

	int (*get_block)(struct generic_drive *drive, int index, u8 buff[]);
	int (*put_block)(struct generic_drive *drive, int index, const u8 buff[]);
};

int generic_drive_register(struct generic_drive *drive);
