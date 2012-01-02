#pragma once

#include <types.h>
#include <block.h>

struct file;
struct dentry;
struct inode;
struct block_device;
struct vfsmount;

struct file_system_type {
	const char *name;
	const struct file_operations *fops;
	struct file_system_type *next;

	struct dentry *(*mount)(struct file_system_type *, unsigned long, struct block_device *);
	struct dentry *(*lookup)(struct inode *, const char *);
};

int file_system_type_register(struct file_system_type *);

struct file_system_type *file_system_type_get(const char *);

struct vfsmount {
	const char *path;
	struct block_device *bdev;
	struct file_system_type *fs_type;
	struct list_node mnt_hash;
	// void *fs_ext;
	struct dentry *root;
};

struct block_buff {
	// __u32  blk_id;
	__u8    *blk_base;
	size_t  blk_size;
	size_t  max_size;
	__u8    *blk_off;
};

struct file_operations {
	int (*open)(struct file *, struct inode *);
	int (*close)(struct file *);
	ssize_t (*read)(struct file *, void *, size_t, loff_t *);
	ssize_t (*write)(struct file *, const void *, size_t, loff_t *);
	int (*ioctl)(struct file *, int, unsigned long);
	loff_t (*lseek)(struct file *, loff_t, int);
};

struct file {
	size_t pos;
	unsigned int flags;
	// unsigned int mode;
	const struct file_operations *fops;
	struct dentry *de;
	// struct vfsmount *vfsmnt;
	struct block_buff blk_buf; // for block device, to be removed!
	void *private_data;
};

#define IMODE_R   (1 << 0)
#define IMODE_W   (1 << 1)
#define IMODE_X   (1 << 2)

struct inode {
	__u32 mode;
	void *i_fs; // fixme!
	void *i_ext;
};

struct dentry {
	struct inode *inode;
	void *d_ext;
};

int device_create(struct list_node *node);
