#include <stdio.h>
#include <init.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <block.h>
#include <dirent.h>
#include <fs/fs.h>

#define RAM_BLK_SIZE   (1 << 12)

struct ram_super_block {
	struct list_node *r_list;
};

struct ram_inode {
	struct inode vfs_inode;
	union {
		void *data;
	};
};

// fixme
static DECL_INIT_LIST(g_root_dir);

static int ram_open(struct file *fp, struct inode *inode);
static int ram_close(struct file *fp);
static ssize_t ram_read(struct file *fp, void *buff, size_t size, loff_t *off);
static ssize_t ram_write(struct file *fp, const void *buff, size_t size, loff_t *off);

static const struct inode_operations ram_reg_inode_operations = {
};

static const struct file_operations ram_reg_file_operations = {
	.open  = ram_open,
	.close = ram_close,
	.read  = ram_read,
	.write = ram_write,
};

static struct dentry *ram_lookup(struct inode *parent, struct dentry *dentry,
	struct nameidata *nd);
static int ram_mkdir(struct inode *, struct dentry *, int);

static const struct inode_operations ram_dir_inode_operations = {
	.lookup = ram_lookup,
	.mkdir  = ram_mkdir,
};

static int ram_opendir(struct file *fp, struct inode *inode);
static int ram_readdir(struct file *, struct linux_dirent *);

static const struct file_operations ram_dir_file_operations = {
	.open    = ram_opendir,
	.readdir = ram_readdir,
};

static inline struct ram_inode *RAM_I(struct inode *inode)
{
	return container_of(inode, struct ram_inode, vfs_inode);
}

static struct inode *ram_alloc_inode(struct super_block *sb)
{
	struct ram_inode *rin;

	rin = zalloc(sizeof(*rin));
	if (!rin)
		return NULL;

	rin->vfs_inode.i_sb = sb; // fixme

	return &rin->vfs_inode;
}

static inline struct ram_inode *ram_get_inode(struct super_block *sb, int ino)
{
	return NULL;
}

struct inode *ram_inode_create(struct super_block *sb, int mode)
{
	struct inode *inode;
	// struct ram_inode *rin;

	inode = ram_alloc_inode(sb);
	if (!inode) {
		// ...
		return NULL;
	}

	inode->i_ino  = 12345;
	inode->i_mode = mode;

	// rin = RAM_I(inode);

	if (S_ISREG(inode->i_mode)) {
		inode->i_op = &ram_reg_inode_operations;
		inode->i_fop = &ram_reg_file_operations;
	} else if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &ram_dir_inode_operations;
		inode->i_fop = &ram_dir_file_operations;
	} else {
		BUG();
	}

	return inode;
}

struct inode *ram_iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;
	// struct ram_inode *rin;

	inode = ram_alloc_inode(sb);
	if (!inode) {
		// ...
		return NULL;
	}

	inode->i_ino  = ino;

	// rin = RAM_I(inode);

	if (S_ISREG(inode->i_mode)) {
		inode->i_op = &ram_reg_inode_operations;
		inode->i_fop = &ram_reg_file_operations;
	} else if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &ram_dir_inode_operations;
		inode->i_fop = &ram_dir_file_operations;
	} else {
		BUG();
	}

	return inode;
}

static int ram_fill_super(struct super_block *sb)
{
	int ret;
	struct ram_super_block *ram_sb;

	ram_sb = zalloc(sizeof(*ram_sb));
	if (!ram_sb) {
		ret = -ENOMEM;
		goto L1;
	}

	sb->s_fs_info = ram_sb;
	sb->s_blksize = RAM_BLK_SIZE;

	return 0;
L1:
	return ret;
}

static struct dentry *ram_mount(struct file_system_type *fs_type, unsigned long flags, const char *bdev_name)
{
	int ret;
	struct dentry *root;
	struct inode *in;
	struct super_block *sb;

	sb = sget(fs_type, NULL);
	if (!sb)
		return NULL;

	ret = ram_fill_super(sb);
	if (ret < 0) {
		// ...
		return NULL;
	}

	in = ram_inode_create(sb, S_IFDIR);
	if (!in) {
		// ...
		return NULL;
	}

	root = d_alloc_root(in);
	if (!root)
		return NULL;

	sb->s_root = root;

	return root;
}

// fixme
static void ram_umount(struct super_block *sb)
{
}

static int ram_open(struct file *fp, struct inode *inode)
{
	return 0;
}

static int ram_close(struct file *fp)
{
	return 0;
}

static ssize_t ram_read(struct file *fp, void *buff, size_t size, loff_t *off)
{
	return 0;
}

static ssize_t
ram_write(struct file *fp, const void *buff, size_t size, loff_t *off)
{
	return 0;
}

static struct dentry *
ram_lookup(struct inode *parent, struct dentry *dentry, struct nameidata *nd)
{
	nd->ret = -ENOENT;
	return NULL;
}

static int ram_opendir(struct file *fp, struct inode *inode)
{
	fp->private_data = fp->f_dentry->d_subdirs.next;
	return 0;
}

static int ram_readdir(struct file *fp, struct linux_dirent *dirent)
{
	struct dentry *de;
	struct inode *in;
	struct list_node *iter;

	if (fp->f_pos >= fp->f_dentry->d_inode->i_size)
		return 0;

	iter = fp->private_data;
	de = container_of(iter, struct dentry, d_child);

	in = de->d_inode;
	// TODO: fix the offset and type
	filldir(dirent, de->d_name.name, de->d_name.len, 0, in->i_ino, in->i_mode);

	fp->private_data = iter->next;
	fp->f_pos++;

	return dirent->d_reclen;
}

static int ram_mkdir(struct inode *parent, struct dentry *de, int mode)
{
	struct inode *inode;

	inode = ram_alloc_inode(parent->i_sb);
	if (!inode)
		return -ENOMEM;

	inode->i_ino = 1234; // fixme
	inode->i_mode = mode;

	inode->i_op = &ram_dir_inode_operations;
	inode->i_fop = &ram_dir_file_operations;

	de->d_inode = inode;

	parent->i_size++;

	return 0;
}

static struct file_system_type ram_fs_type = {
	.name   = "tmpfs",
	.mount  = ram_mount,
	.umount = ram_umount,
};

static int __INIT__ ramfs_init(void)
{
	return file_system_type_register(&ram_fs_type);
}

module_init(ramfs_init);
