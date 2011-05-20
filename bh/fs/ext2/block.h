#pragma once

#include <linux/types.h>

#define DISK_BLOCK_SIZE	 512

struct block_device
{
	const char *name;
	int fd;

	ssize_t (*get_block)(struct block_device *, int, void *);
};

struct block_device *bdev_open(const char *name);

int bdev_close(struct block_device *bdev);
