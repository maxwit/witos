#pragma once

struct block_device
{
	size_t dev_size;
	size_t blk_size;

	int (*get_block)(struct block_device *blkdev, int idx, u8 buff[]);
	int (*put_block)(struct block_device *blkdev, int idx, const u8 buff[]);
};

int block_device_register(struct block_device *blkdev);
