#include <stdio.h>
#include <string.h>
#include <block.h>
#include <list.h>

static DECL_INIT_LIST(g_bdev_list);

struct block_device *get_bdev_by_name(const char *name)
{
	struct list_node *iter;
	struct block_device *bdev;

	list_for_each(iter, &g_bdev_list)
	{
		bdev = container_of(iter, struct block_device, bdev_node);
		if (!strcmp(bdev->name, name))
			return bdev;
	}

	return NULL;
}

struct block_device *get_bdev_by_volume(char vol)
{
	struct list_node *iter;
	struct block_device *bdev;

	if (vol >= 'a')
		vol -= 'a' - 'A';

	assert(vol >= 'A' && vol <= 'Z');

	list_for_each(iter, &g_bdev_list) {
		bdev = container_of(iter, struct block_device, bdev_node);
		if (bdev->volume == vol)
			return bdev;
	}

	return NULL;
}

int block_device_register(struct block_device *bdev)
{
	static char vol = 'A';

	if (vol > 'Z')
		return -EBUSY;

	bdev->volume = vol++;
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
