#pragma once

#include <list.h>
#include <block.h>

struct generic_drive
{
	struct block_device blk_dev;

	int (*get_block)(struct generic_drive *drive, int start, void *buff);
	int (*put_block)(struct generic_drive *drive, int start, const void *buff);

	union
	{
		struct list_node master_node;
		struct list_node slave_node;
	};

	union
	{
		struct list_node slave_list;
		struct generic_drive *master;
	};
};

int generic_drive_register(struct generic_drive *drive);
