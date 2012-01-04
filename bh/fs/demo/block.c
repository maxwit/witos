#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <block.h>
#include <stdlib.h>

#define SECT_SIZE   (1 << 9)

static DECL_INIT_LIST(g_bdev_list);

const struct list_node *bdev_get_list()
{
	return &g_bdev_list;
}

int block_device_register(struct block_device *bdev)
{
	int fd;

	fd = open(bdev->name, O_RDONLY);
	if (fd < 0) {
		return 0;
	}

	bdev->fd = fd;

	list_add_tail(&bdev->bdev_node, &g_bdev_list);	

#if 0
	printf("    0x%08x - 0x%08x %s (%s)\n",
		bdev->base, bdev->base + bdev->size, bdev->name,
		bdev->label[0] ? bdev->label : "N/A");
#endif

	return 0;
}

struct bio *bio_alloc(/* reserved */)
{
	struct bio *bio;

	bio = malloc(sizeof(*bio));
	if (bio)
		memset(bio, 0, sizeof(*bio));

	return bio;
}

void bio_free(struct bio *bio)
{
	free(bio);
}

void submit_bio(int rw, struct bio *bio)
{
	int ret;
	struct block_device *bdev = bio->bdev;

	lseek(bdev->fd, bio->sect << 9, SEEK_SET);

	if (READ == rw)
		ret = read(bdev->fd, bio->data, bio->size);
	else
		ret = write(bdev->fd, bio->data, bio->size);

	if (ret < 0)
		bio->flags = 1; // fixme
}

void *zalloc(size_t sz)
{
	void *p;

	p = malloc(sz);
	if (p)
		memset(p, 0, sz);

	return p;
}
