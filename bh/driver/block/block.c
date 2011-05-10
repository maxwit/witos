#include <block.h>

int block_device_register(struct block_device *blk_dev)
{
	printf("%s(): registering %s\n", __func__, blk_dev->dev.name);

	return 0;
}
