#include <stdio.h>
#include <string.h>
#include <block.h>
#include <list.h>

static DECL_INIT_LIST(g_bdev_list);

const struct list_node *bdev_get_list()
{
	return &g_bdev_list;
}

int block_device_register(struct block_device *bdev)
{
	list_add_tail(&bdev->bdev_node, &g_bdev_list);

	printf("    0x%08x - 0x%08x %s (%s)\n",
		bdev->base, bdev->base + bdev->size, bdev->name,
		bdev->label[0] ? bdev->label : "N/A");

	return 0;
}

static int __INIT__ block_device_init(void)
{
	return 0;
}

SUBSYS_INIT(block_device_init);
