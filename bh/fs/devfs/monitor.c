#include <list.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <block.h>
#include <fs/fs.h>
#include <fs/devfs.h>

static DECL_INIT_LIST(g_device_queue);

int device_enqueue(struct block_device *bdev)
{
	struct qdev_node *node;

	node = malloc(sizeof(*node)); //
	// if null ...

	node->data = bdev;
	list_add_tail(&node->dev_node, &g_device_queue);

	return 0;
}

static struct block_device *device_dequeue()
{
	struct list_node *first;
	struct block_device *bdev;
	struct qdev_node *node;

	first = g_device_queue.next;
	list_del_node(first);
	node = container_of(first, struct qdev_node, dev_node);
	bdev = node->data;

	free(node);
	return bdev;
}

int device_monitor()
{
	int ret;
	struct qstr name;
	struct nameidata nd;
	struct dentry *parent;

	ret = path_walk("/dev" /* DEV_ROOT */, &nd);
	if (ret < 0)
		return ret;

	parent = nd.path.dentry;

	while (!list_is_empty(&g_device_queue)) {
		struct block_device *dev;
		struct dentry *dentry;

		dev = device_dequeue(); // fixme: get first instead
		name.name = dev->name;
		name.len  = strlen(dev->name);

		dentry = d_alloc(parent, &name);
		if (!dentry)
			return -ENOMEM;

		ret = vfs_mknod(parent->d_inode, dentry, 0666 | S_IFBLK);
		if (ret < 0) {
			GEN_DBG("fail to create \"%s\" (errno = %d)!\n", dev->name, ret);
			return ret;
		}
	}

	return 0;
}
