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
		// struct list_node list;
	};
};

// fixme
static DECL_INIT_LIST(g_root_dir);

static struct dentry *ram_lookup(struct inode *parent, struct dentry *dentry,
	struct nameidata *nd);
static int ram_open(struct file *fp, struct inode *inode);
static int ram_close(struct file *fp);
static ssize_t ram_read(struct file *fp, void *buff, size_t size, loff_t *off);
static ssize_t ram_write(struct file *fp, const void *buff, size_t size, loff_t *off);
static int ram_readdir(struct file *, struct linux_dirent *);
static int ram_mkdir(struct inode *, struct dentry *, int);

static const struct inode_operations ram_reg_inode_operations = {
};

static const struct file_operations ram_reg_file_operations = {
	.open  = ram_open,
	.close = ram_close,
	.read  = ram_read,
	.write = ram_write,
};

static const struct inode_operations ram_dir_inode_operations = {
	.lookup = ram_lookup,
	.mkdir  = ram_mkdir,
};

static const struct file_operations ram_dir_file_operations = {
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

static struct ram_inode *ram_get_inode(struct super_block *sb, int ino)
{
	return NULL;
}

struct inode *ram_iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;
	struct ram_inode *rin;

	inode = ram_alloc_inode(sb);
	if (!ino) {
		// ...
		return NULL;
	}

	inode->i_ino  = ino;

	rin = RAM_I(inode);

	rin->data = NULL;
	if (S_ISREG(inode->i_mode)) {
		inode->i_op = &ram_reg_inode_operations;
		inode->i_fop = &ram_reg_file_operations;
	} else { // if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &ram_dir_inode_operations;
		inode->i_fop = &ram_dir_file_operations;
	}
#if 0
	else {
		BUG();
	}
#endif

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

	in = ram_iget(sb, 1);
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
	struct ram_inode *rin = container_of(inode, struct ram_inode, vfs_inode);

	fp->private_data = rin->data;

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

static ssize_t ram_write(struct file *fp, const void *buff, size_t size, loff_t *off)
{
	return 0;
}

static struct dentry *ram_lookup(struct inode *parent, struct dentry *dentry,
	struct nameidata *nd)
{
#if 0
	unsigned long ino;
	struct inode *inode;
	struct dentry *dir, *child;
	struct ram_inode *rin;
	struct list_node *iter;

	rin = RAM_I(parent);
	dir = rin->data;

	//
#endif
	nd->ret = -ENOENT;
	return NULL;
}

static int ram_readdir(struct file *fp, struct linux_dirent *dirent)
{
	struct inode *inode;
	struct ram_inode *rin;
	struct linux_dirent *rde;

	inode = fp->f_dentry->d_inode;
	if (fp->f_pos >= inode->i_size)
		return 0;

	rin = RAM_I(inode);
	rde = rin->data + fp->f_pos;
	memcpy(dirent, rde, rde->d_reclen);

	fp->f_pos += rde->d_reclen;
	return rde->d_reclen;
}

static int ram_mkdir(struct inode *parent, struct dentry *de, int mode)
{
	struct inode *cur_in;
	struct ram_inode *par_rin;
	struct linux_dirent lde;

	cur_in = ram_alloc_inode(parent->i_sb);
	//
	cur_in->i_ino = 1234; // fixme
	cur_in->i_mode = mode;

	cur_in->i_op = &ram_dir_inode_operations;
	cur_in->i_fop = &ram_dir_file_operations;

	de->d_inode = cur_in;

	par_rin = RAM_I(parent);
	if (!par_rin->data) {
		par_rin->data = malloc(RAM_BLK_SIZE);
		if (!par_rin->data)
			return -ENOMEM;
	}

	// fixme
	filldir(&lde, de->d_name.name, de->d_name.len,
		0, de->d_inode->i_ino, 0);

	if (parent->i_size + lde.d_reclen >= RAM_BLK_SIZE) {
		GEN_DBG("overflow!\n");
		return -EOVERFLOW;
	}

	memcpy(par_rin->data + parent->i_size, &lde, lde.d_reclen);
	parent->i_size += lde.d_reclen;

	return 0;
}

static struct file_system_type ram_fs_type = {
	.name   = "tmpfs",
	.mount  = ram_mount,
	.umount = ram_umount,
};

static int __INIT__ ram_init(void)
{
	return file_system_type_register(&ram_fs_type);
}

module_init(ram_init);
