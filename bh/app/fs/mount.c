#include <fs.h>

int main(int argc, char *argv[])
{
	struct list_node *iter, *list = get_bdev_list();
	struct block_device *bdev;

	if (argc != 2)
	{
		printf("Usage:\n\t%s device\n", argv[1]);
		return -EINVAL;
	}

	list_for_each(iter, list)
	{
		bdev = container_of(iter, struct block_device, bdev_node);
		if (!strcmp(argv[1], bdev->dev.name))
			break;
	}

	fat_mount(bdev, "fat", 0);

	return 0;
}
