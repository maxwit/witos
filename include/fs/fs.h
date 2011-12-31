#pragma once

#include <types.h>
#include <block.h>

struct file_operations;
struct file_system;
struct dentry;

struct file_system_type {
	const char *name;
	const struct file_operations *fops;
	struct file_system_type *next;

	int (*mount)(struct file_system_type *, unsigned long, struct block_device *);

	// fixme
	struct dentry *(*lookup)(struct file_system *, const char *name);
};

int file_system_type_register(struct file_system_type *);

struct file_system_type *file_system_type_get(const char *);

struct mount_point {
	const char *path;
	struct block_device *bdev;
	struct file_system_type *fs_type;
	struct mount_point *next;
};

struct block_device;

struct block_buff {
	// __u32  blk_id;
	__u8   *blk_base;
	size_t  blk_size;
	size_t  max_size;
	__u8  *blk_off;
};

struct dentry;

struct file {
	// common
	size_t pos;
	int mode;
	const struct file_operations *fops;

	// for regular file
	struct dentry *de; // dir entry
	// void *fs; // file system
	struct mount_point *mnt;

	// for block device
	struct block_device *bdev;
	struct block_buff blk_buf; // to be removed!
};

struct file_operations {
	int (*open)(struct file *, int, int);
	int (*close)(struct file *);

	ssize_t (*read)(struct file *, void *, size_t, loff_t *);
	ssize_t (*write)(struct file *, const void *, size_t, loff_t *);

	int (*ioctl)(struct file *, int, unsigned long);
	loff_t (*lseek)(struct file *, loff_t, int);
};

struct dentry {
	void *ext;
};

struct file_system {
	void *ext;
};
