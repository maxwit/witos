#pragma once

#include <list.h>
#include <block.h>

struct disk_drive {
	struct block_device bdev;

	size_t sect_size;

	int (*get_block)(struct disk_drive *drive, int start, void *buff);
	int (*put_block)(struct disk_drive *drive, int start, const void *buff);

	union {
		struct list_node master_node;
		struct list_node slave_node;
	};

	union {
		struct list_node slave_list;
		struct disk_drive *master;
	};
};

int disk_drive_register(struct disk_drive *drive);
