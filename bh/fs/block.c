#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "block.h"
#include "ext2.h"

struct block_device *bdev_open(const char *name)
{
	int fd;
	struct block_device *bdev;

	fd = open(name, O_RDWR);
	if (fd < 0)
	{
		return NULL;
	}

	bdev = malloc(sizeof(*bdev));

	bdev->name = name;
	bdev->fd   = fd;

	return bdev;
}

int bdev_close(struct block_device *bdev)
{
	int ret;

	ret = close(bdev->fd);
	free(bdev);

	return ret;
}

ssize_t bdev_read_block(struct block_device *bdev, int blk_no, size_t off, void *buff, size_t size)
{
#if 0
	char blk_buff[DISK_BLOCK_SIZE];

	lseek(bdev->fd, blk_no * DISK_BLOCK_SIZE, SEEK_SET);

	read(bdev->fd, blk_buff, DISK_BLOCK_SIZE);

	memcpy(buff, blk_buff + off, size);

	return size;
#else
	lseek(bdev->fd, blk_no * DISK_BLOCK_SIZE + off, SEEK_SET);
	return read(bdev->fd, buff, size);
#endif
}
	// memcpy(&sblk, buff, sizeof(sblk));
	// printf("sizeof(sblk) = %d\n", sizeof(sblk));
	// printf("%x %s\n", sblk.s_magic, sblk.s_volume_name);
