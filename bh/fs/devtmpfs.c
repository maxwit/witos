#include <fs/fs.h>

static DECL_INIT_LIST(g_dev_list);

int device_create(struct list_node *node)
{
	list_add_tail(node, &g_dev_list);
	return 0;
}

static struct dentry *devfs_mount(struct file_system_type *fstype, unsigned long flags, struct block_device *bdev)
{
	struct dentry *root;
	struct inode *ino;

	ino = zalloc(sizeof(*ino));
	if (!ino)
		return NULL;

	ino->mode = ~0;

	root = zalloc(sizeof(*root));
	if (!root)
		return NULL;

	root->inode = ino;

	return root;
}

struct dentry *devfs_lookup(struct inode *parent, const char *name)
{
	struct list_node *iter;

	list_for_each(iter, &g_dev_list) {
		struct block_device *bdev;

		bdev = container_of(iter, struct block_device, devfs_node);
		if (!strcmp(bdev->name, name)) {
			struct dentry *de;
			struct inode *inode;

			// init inode here
			inode = zalloc(sizeof(*inode));
			if (!inode)
				return NULL;

			if (bdev->flags == BDF_RDONLY)
				inode->mode = IMODE_R;
			else
				inode->mode = ~0; // fixme

			inode->i_ext = bdev;
			inode->i_sb = parent->i_sb;

			de = zalloc(sizeof(*de));
			if (!de)
				return NULL;

			de->inode = inode;

			return de;
		}
	}

	return NULL;
}

extern const struct file_operations flash_fops;

static struct file_system_type devfs_type = {
	.name   = "devfs",
	.fops   = &flash_fops,
	.mount  = devfs_mount,
	.lookup = devfs_lookup,
};

static int __INIT__ devfs_init(void)
{
	return file_system_type_register(&devfs_type);
}

SUBSYS_INIT(devfs_init);
