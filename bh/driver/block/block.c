#include <block.h>
#include <list.h>

static struct list_node g_bdev_list;

struct list_node *get_bdev_list(void)
{
	return &g_bdev_list;
}

int block_device_register(struct block_device *blk_dev)
{
	printf("%s(): registering %s\n", __func__, blk_dev->dev.name);

	list_add_tail(&blk_dev->bdev_node, &g_bdev_list);

	return 0;
}

static int __INIT__ block_device_init(void)
{
	list_head_init(&g_bdev_list);

	return 0;
}

SUBSYS_INIT(block_device_init);