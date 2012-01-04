#include <types.h>

typedef unsigned long sector_t;

struct block_device {
	int fd;
};

struct bio {
	void     *data;
	size_t   size;
	sector_t sect;
	struct block_device *bdev;
	unsigned long flags;
};

struct bio *bio_alloc();

void bio_free(struct bio *bio);

void submit_bio(int rw,struct bio * bio);
