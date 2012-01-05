#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <fs/fs.h>

#define MAX_FDS 256

struct qstr {
	const char *name;
	unsigned int len;
};

struct nameidata {
	unsigned int flags;
	struct file *fp;
};

static struct file_system_type *fs_type_list;
static DECL_INIT_LIST(g_mount_list);
static struct file *fd_array[MAX_FDS];

int file_system_type_register(struct file_system_type *fs_type)
{
	struct file_system_type **p;

	for (p = &fs_type_list; *p; p = &(*p)->next) {
		if (!strcmp((*p)->name, fs_type->name))
			return -EBUSY;
	}

	fs_type->next = NULL;
	*p = fs_type;

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
	struct file_system_type *fs_type;
	struct vfsmount *mnt;
	struct dentry *root;

	// fixme
	struct block_device *bdev;
	bdev = bdev_get(bdev_name);
	if (NULL == bdev) {
		DPRINT("fail to open block device \"%s\"!\n", bdev_name);
		return -ENODEV;
	}

	fs_type = file_system_type_get(type);
	if (NULL == fs_type) {
		DPRINT("fail to find file system type %s!\n", type);
		return -ENOENT;
	}

	root = fs_type->mount(fs_type, flags, bdev);
	if (!root) {
		DPRINT("fail to mount %s!\n", bdev_name);
		goto L1;
	}

	mnt = malloc(sizeof(*mnt));
	if (NULL == mnt) {
		ret = -ENOMEM;
		goto L2;
	}

	mnt->fs_type = fs_type;
	mnt->path = path;
	mnt->bdev = bdev;
	mnt->root = root;

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
		if (!strncmp(mnt->path, unit->name, unit->len))
			return mnt;
	}

	return NULL;
}

static int path_walk(const char *path, struct nameidata *nd)
{
	int ret = -ENOENT; // fixme
	struct qstr str;
	struct file *fp;
	struct vfsmount *mnt;
	struct file_system_type *fs_type;
	struct dentry *de, *root;
	struct inode *inode;

	while ('/' == *path)
		path++;
	if (!*path)
		return -EINVAL;
	str.name = path;

	while (*path && '/' != *path)
		path++;
	if (!*path)
		return -EINVAL;
	str.len = path - str.name;

	mnt = search_mount(&str);
	if (NULL == mnt)
		return -ENOENT;

	root = mnt->root;
	fs_type = mnt->fs_type;
	if (!fs_type->lookup || !fs_type->fops)
		return -EINVAL;

	while ('/' == *path)
		path++;
	if (!*path)
		return -EINVAL;

	de = fs_type->lookup(root->inode, path);
	if (!de) {
		return -ENOENT;
	}

	inode = de->inode;
	if (O_RDONLY != nd->flags && !(inode->mode & (IMODE_W | IMODE_X)))
		return -EPERM;

	fp = zalloc(sizeof(*fp)); // use malloc instead of zalloc
	if (!fp) {
		ret = -ENOMEM;
		goto no_mem;
	}

	fp->de = de;
	fp->pos = 0;
	fp->fops = fs_type->fops;
	fp->flags = nd->flags;

	if (!fp->fops) {
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

	if (fp->fops->open) {
		ret = fp->fops->open(fp, fp->de->inode);
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

	return fp->fops->close(fp);
}

EXPORT_SYMBOL(close);

ssize_t GAPI read(int fd, void *buff, size_t size)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->fops->read)
		return -ENOENT;

	return fp->fops->read(fp, buff, size, &fp->pos);
}

EXPORT_SYMBOL(read);

ssize_t GAPI write(int fd, const void *buff, size_t size)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->fops->write)
		return -ENOENT;

	return fp->fops->write(fp, buff, size, &fp->pos);
}

EXPORT_SYMBOL(write);

int GAPI ioctl(int fd, int cmd, ...)
{
	struct file *fp;
	unsigned long arg;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->fops->ioctl)
		return -ENOENT;

	arg = *((unsigned long *)&cmd + 1); // fixme

	return fp->fops->ioctl(fp, cmd, arg);
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

	if (fp->fops->lseek)
		return fp->fops->lseek(fp, offset, whence);

	switch (whence) {
	case SEEK_SET:
		fp->pos = offset;
		break;

	case SEEK_CUR:
		fp->pos += offset;
		break;

	case SEEK_END:
	default:
		return -EINVAL;
	}

	return 0;
}

EXPORT_SYMBOL(lseek);
