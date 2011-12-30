#pragma once

struct block_device;

// fixme: to be removed
struct block_buff {
	// __u32  blk_id;
	__u8   *blk_base;
	size_t  blk_size;
	size_t  max_size;
	__u8  *blk_off;
};

struct bdev_fops {
	int (*open)(struct bdev_file *, int, int);
	int (*close)(struct bdev_file *);

	ssize_t (*read)(struct bdev_file *, void *, size_t, loff_t *);
	ssize_t (*write)(struct bdev_file *, const void *, size_t, loff_t *);

	int (*ioctl)(struct bdev_file *, int, unsigned long);
	int (*lseek)(struct bdev_file *, loff_t, int);
};

struct bdev_file {
	// struct block_device *bdev;
	void *ext;
	size_t cur_pos;

	struct bdev_fops *fops;

	// to be removed!
	struct block_buff blk_buf;
	char name[FILE_NAME_SIZE];
	size_t size;
};

int bdev_open(const char *name, int flags, ...);
int bdev_close(int fd);
ssize_t bdev_read(int fd, void *buff, size_t size);
ssize_t bdev_write(int fd, const void * buff, size_t size);
int bdev_ioctl(int fd, int cmd, ...);
