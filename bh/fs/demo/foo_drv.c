#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <drive.h>

#define BLK_LEN 512

struct foo_drive
{
	struct disk_drive drive;
	int fd;
};

static int foo_card_count = 0;

static int foo_get_block(struct disk_drive *drive, int start, void *buff)
{
	struct foo_drive *foo = container_of(drive, struct foo_drive, drive);

	lseek(foo->fd, start * BLK_LEN, SEEK_SET);
	read(foo->fd, buff, BLK_LEN);

	return BLK_LEN;
}

static int foo_put_block(struct disk_drive *drive, int start, const void *buff)
{
	struct foo_drive *foo = container_of(drive, struct foo_drive, drive);

	lseek(foo->fd, start, SEEK_SET);
	write(foo->fd, buff, BLK_LEN);

	return BLK_LEN;
}

static int foo_card_register(struct foo_drive *foo)
{
	struct disk_drive *drive = &foo->drive;

	sprintf(drive->bdev.dev.name, "foo%d", foo_card_count++);

	// TODO: fix size
	drive->bdev.bdev_base = 0;
	drive->bdev.bdev_size = 0;
	drive->bdev.sect_size = 512;

	list_head_init(&drive->slave_list);

	drive->get_block = foo_get_block;
	drive->put_block = foo_put_block;

	return disk_drive_register(drive);
}

static struct foo_drive foo;

int foo_drv_init(const char *fn)
{
	int fd;
	int perror(const char *);

	fd = open(fn, O_RDWR);
	if (fd < 0)
	{
		perror(fn);
		return fd;
	}

	memset(&foo, 0, sizeof(foo));
	foo.fd = fd;

	return foo_card_register(&foo);
}
