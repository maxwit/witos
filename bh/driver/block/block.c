#include <stdio.h>
#include <string.h>
#include <block.h>
#include <list.h>
#include <fs/fs.h>
#include <drive.h> // fixme

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

	device_create(&bdev->devfs_node);

	return 0;
}

struct bio *bio_alloc(/* reserved */)
{
	struct bio *bio;

	bio = zalloc(sizeof(*bio));

	return bio;
}

void bio_free(struct bio *bio)
{
	free(bio);
}

void submit_bio(int rw, struct bio *bio)
{
	int ret;
	size_t len;
	sector_t sect = bio->sect;
	struct block_device *bdev = bio->bdev;
	struct disk_drive *drive;

	assert(bdev);

	drive = container_of(bdev, struct disk_drive, bdev);

	for (len = 0; len < bio->size; len += 512) {
		if (READ == rw)
			ret = drive->get_block(drive, sect << 9, bio->data + len);
		else
			ret = drive->put_block(drive, sect << 9, bio->data + len);

		sect++;
	}

	if (ret < 0)
		bio->flags = 1; // fixme
}

static int __INIT__ block_device_init(void)
{
	return 0;
}

SUBSYS_INIT(block_device_init);
