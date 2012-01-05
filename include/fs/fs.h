#pragma once

#include <types.h>
#include <list.h>
#include <block.h>

struct vfsmount;
struct super_block;
struct inode;
struct dentry;
struct file;

struct qstr {
	const char *name;
	unsigned int len;
};

struct nameidata {
	struct qstr *unit;
	unsigned int flags;
	struct file *fp;
};

struct file_system_type {
	const char *name;
	struct file_system_type *next;

	struct dentry *(*mount)(struct file_system_type *, unsigned long, struct block_device *);
	void (*umount)(struct super_block *);

	// fixme: to remove the followings
	const struct file_operations *fops;
	struct dentry *(*lookup)(struct inode *, struct nameidata *nd);
};

int file_system_type_register(struct file_system_type *);

struct file_system_type *file_system_type_get(const char *);

struct vfsmount {
	const char *dev_name;
	const char *mountpoint; // fixme: dentry type
	struct dentry *root;
	struct file_system_type *fstype;
	struct list_node mnt_hash;
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
	loff_t f_pos;
	unsigned int flags;
	// unsigned int mode;
	const struct file_operations *f_op;
	struct dentry *f_dentry;
	// struct vfsmount *vfsmnt;
	struct block_buff blk_buf; // for block device, to be removed!
	void *private_data;
};

#define IMODE_R   (1 << 0)
#define IMODE_W   (1 << 1)
#define IMODE_X   (1 << 2)

struct inode {
	__u32 mode;
	struct super_block *i_sb;
	void *i_ext;
};

struct dentry {
	// char d_name[NAME_SIZE];
	struct inode *inode;
	struct super_block *d_sb;
	void *d_ext;
};

struct super_block {
	struct block_device *s_bdev;
	struct dentry *s_root;
	void *s_ext;
};

int device_create(struct list_node *node);
