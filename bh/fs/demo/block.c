#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <block.h>
#include <stdlib.h>

#define SECT_SIZE 512

enum {
	READ = 1,
	WRITE,
};

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
	size_t len;
	sector_t sect = bio->sect;
	struct block_device *bdev = bio->bdev;

	for (len = 0; len < bio->size; len += SECT_SIZE) {
		if (READ == rw)
			ret = read(bdev->fd, sect << 9, bio->data + len);
		else
			ret = write(bdev->fd, sect << 9, bio->data + len);

		sect++;
	}

	if (ret < 0)
		bio->flags = 1; // fixme
}
