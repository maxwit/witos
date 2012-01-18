#include <list.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <block.h>
#include <syscalls.h>
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

// TODO: run as a delay work in app layer
static int bdev_mount(const char *dev_name)
{
	int ret;
	static char mnt[] = "/d";

	// to be removed
	if (!strncmp(dev_name, "/dev/mtdblock", sizeof("/dev/mtdblock") - 1) || \
		!strncmp(dev_name, "mtdblock", sizeof("mtdblock") - 1))
		return -ENOTSUPP;

	GEN_DBG("mounting %s -> %s\n", dev_name, mnt);

	ret = sys_mkdir(mnt, 0777);
	if (ret < 0) {
		// ...
		return ret;
	}

	ret = sys_mount(dev_name, mnt, "ext2", 0); // fixme: try other fstypes
	if (ret < 0) {
		printf("fail to mount %s (errno = %d)!\n", dev_name, ret);
		return ret;
	}

	mnt[1]++; // fix when mnt[1] > z

	return 0;
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

		bdev_mount(dev->name);
	}

	return 0;
}
