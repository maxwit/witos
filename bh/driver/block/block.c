#include <stdio.h>
#include <string.h>
#include <block.h>
#include <list.h>

static struct list_node g_bdev_list;

struct block_device *get_bdev_by_name(const char *name)
{
	struct list_node *iter;
	struct block_device *bdev;

	list_for_each(iter, &g_bdev_list)
	{
		bdev = container_of(iter, struct block_device, bdev_node);
		if (!strcmp(bdev->dev.name, name))
			return bdev;
	}

	return NULL;
}

int block_device_register(struct block_device *bdev)
{
	// printf("%s(): registering %s\n", __func__, bdev->dev.name);
	list_add_tail(&bdev->bdev_node, &g_bdev_list);

	return 0;
}

#ifdef CONFIG_HOST_DEMO
int block_device_init(void)
#else
static int __INIT__ block_device_init(void)
#endif
{
	list_head_init(&g_bdev_list);

	return 0;
}

#ifndef CONFIG_HOST_DEMO
SUBSYS_INIT(block_device_init);
#endif
