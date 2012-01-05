#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <fs/fs.h>

#define MAX_FDS 256

static struct file_system_type *fs_type_list;
static DECL_INIT_LIST(g_mount_list);
static struct file *fd_array[MAX_FDS];

int file_system_type_register(struct file_system_type *fstype)
{
	struct file_system_type **p;

	for (p = &fs_type_list; *p; p = &(*p)->next) {
		if (!strcmp((*p)->name, fstype->name))
			return -EBUSY;
	}

	fstype->next = NULL;
	*p = fstype;

	return 0;
}

struct file_system_type *file_system_type_get(const char *name)
{
	struct file_system_type *p;

	for (p = fs_type_list; p; p = p->next) {
		if (!strcmp(p->name, name))
			return p;
	}

	return NULL;
}

int GAPI mount(const char *type, unsigned long flags,
	const char *bdev_name, const char *path)
{
	int ret;
	struct file_system_type *fstype;
	struct vfsmount *mnt;
	struct dentry *root;

	// fixme
	struct block_device *bdev;
	bdev = bdev_get(bdev_name);
	if (NULL == bdev) {
		DPRINT("fail to open block device \"%s\"!\n", bdev_name);
		return -ENODEV;
	}

	fstype = file_system_type_get(type);
	if (NULL == fstype) {
		DPRINT("fail to find file system type %s!\n", type);
		return -ENOENT;
	}

	root = fstype->mount(fstype, flags, bdev);
	if (!root) {
		DPRINT("fail to mount %s!\n", bdev_name);
		goto L1;
	}

	mnt = malloc(sizeof(*mnt));
	if (NULL == mnt) {
		ret = -ENOMEM;
		goto L2;
	}

	mnt->fstype     = fstype;
	mnt->mountpoint = path;
	mnt->dev_name   = bdev_name;
	mnt->root       = root;

	// TODO: check if mp exists or not!

	list_add_tail(&mnt->mnt_hash, &g_mount_list);

	return 0;
L2:
	//
L1:
	return ret;
}

EXPORT_SYMBOL(mount);

int GAPI umount(const char *mnt)
{
	return 0;
}

EXPORT_SYMBOL(umount);

static inline struct vfsmount *search_mount(struct qstr *unit)
{
	struct list_node *iter;
	struct vfsmount *mnt;

	list_for_each(iter, &g_mount_list) {
		mnt = container_of(iter, struct vfsmount, mnt_hash);
		if (!strncmp(mnt->mountpoint, unit->name, unit->len))
			return mnt;
	}

	return NULL;
}

#if 0
static inline int path_unit(struct qstr *unit)
{
	const char *path = unit->name;

	while ('/' == *path) path++;
	if (!*path)
		return -EINVAL;
	unit->name = path;

	while (*path && '/' != *path)
		path++;
	if (!*path)
		return -EINVAL;
	unit->len = path - unit->name;

	return unit->len;
}
#endif

static int path_walk(const char *path, struct nameidata *nd)
{
	int ret = -ENOENT; // fixme
	struct qstr unit;
	struct file *fp;
	struct vfsmount *mnt;
	struct file_system_type *fstype;
	struct dentry *child, *parent;
	struct inode *inode;

	while ('/' == *path) path++;
	if (!*path)
		return -EINVAL;
	unit.name = path;

	while (*path && '/' != *path)
		path++;
	if (!*path)
		return -EINVAL;
	unit.len = path - unit.name;

	mnt = search_mount(&unit);
	if (NULL == mnt)
		return -ENOENT;

	child = mnt->root;
	fstype = mnt->fstype;
	if (!fstype->lookup || !fstype->fops)
		return -EINVAL;

	while (1) {
		while ('/' == *path) path++;
		if (!*path)
			break;

		unit.name = path;
		do {
			path++;
		} while (*path && '/' != *path);
		unit.len = path - unit.name;
		GEN_DBG("parsing %s\n", unit.name);

		nd->unit = &unit;
		parent = child;
		child = fstype->lookup(parent->inode, nd);
		if (!child) {
			GEN_DBG("failed @ \"%s\"!\n", unit.name);
			return -ENOENT;
		}
	}

	inode = child->inode;
	if (O_RDONLY != nd->flags && !(inode->mode & (IMODE_W | IMODE_X)))
		return -EPERM;

	fp = zalloc(sizeof(*fp)); // use malloc instead of zalloc
	if (!fp) {
		ret = -ENOMEM;
		goto no_mem;
	}

	fp->f_dentry = child;
	fp->f_pos    = 0;
	fp->f_op     = fstype->fops;
	fp->flags    = nd->flags;

	if (!fp->f_op) {
		GEN_DBG("No fops for %s!\n", path);
		ret = -EINVAL;
		goto fail;
	}

	nd->fp = fp;

	return 0;
fail:
	free(fp);
no_mem:
	return ret;
}

int get_unused_fd()
{
	int fd;

	for (fd = 0; fd < MAX_FDS; fd++) {
		if (!fd_array[fd])
			return fd;
	}

	return -EBUSY;
}

int fd_install(int fd, struct file *fp)
{
	fd_array[fd] = fp;
	return 0;
}

static int do_open(const char *path, int flags, int mode)
{
	int fd, ret;
	struct file *fp;
	struct nameidata nd;

	fd = get_unused_fd();
	if (fd < 0)
		return fd;

	nd.flags = flags;

	ret = path_walk(path, &nd);
	if (ret < 0) {
		GEN_DBG("fail to find \"%s\"!\n", path);
		return ret;
	}

	fp = nd.fp;

	if (fp->f_op->open) {
		ret = fp->f_op->open(fp, fp->f_dentry->inode);
		if (ret < 0)
			goto fail;
	}

	fd_install(fd, fp);

	return fd;

fail:
	free(fp);
	return ret;
}

int GAPI open(const char *path, int flags, ...)
{
	int mode = 0;

	return do_open(path, flags, mode);
}

EXPORT_SYMBOL(open);

int GAPI close(int fd)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp)
		return -ENOENT;

	fd_array[fd] = NULL;

	return fp->f_op->close(fp);
}

EXPORT_SYMBOL(close);

ssize_t GAPI read(int fd, void *buff, size_t size)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->f_op->read)
		return -ENOENT;

	return fp->f_op->read(fp, buff, size, &fp->f_pos);
}

EXPORT_SYMBOL(read);

ssize_t GAPI write(int fd, const void *buff, size_t size)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->f_op->write)
		return -ENOENT;

	return fp->f_op->write(fp, buff, size, &fp->f_pos);
}

EXPORT_SYMBOL(write);

int GAPI ioctl(int fd, int cmd, ...)
{
	struct file *fp;
	unsigned long arg;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->f_op->ioctl)
		return -ENOENT;

	arg = *((unsigned long *)&cmd + 1); // fixme

	return fp->f_op->ioctl(fp, cmd, arg);
}

EXPORT_SYMBOL(ioctl);

loff_t GAPI lseek(int fd, loff_t offset, int whence)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp)
		return -ENOENT;

	if (fp->f_op->lseek)
		return fp->f_op->lseek(fp, offset, whence);

	switch (whence) {
	case SEEK_SET:
		fp->f_pos = offset;
		break;

	case SEEK_CUR:
		fp->f_pos += offset;
		break;

	case SEEK_END:
	default:
		return -EINVAL;
	}

	return 0;
}

EXPORT_SYMBOL(lseek);
