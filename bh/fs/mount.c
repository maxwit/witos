#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <assert.h>
#include <fs/fs.h>

static struct file_system_type *fs_type_list;
static DECL_INIT_LIST(g_mount_list);

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
	static bool root_mounted = false;
	struct file_system_type *fstype;
	struct vfsmount *mnt;
	struct dentry *root;
	struct nameidata nd;

	if ((flags & MS_ROOT) && root_mounted == true)
		return -EBUSY;

	if (!(flags & MS_NODEV)) {
		struct list_node *iter;

		list_for_each(iter, &g_mount_list) {
			mnt = container_of(iter, struct vfsmount, mnt_hash);
			if (!strcmp(mnt->dev_name, bdev_name)) {
				GEN_DBG("device \"%s\" already mounted!\n", bdev_name);
				return -EBUSY;
			}
		}
	}

	if (!(flags & MS_ROOT)) {
		ret = path_walk(path, &nd); // fixme: directory only!
		if (ret < 0) {
			GEN_DBG("\"%s\" does NOT exist!\n");
			return ret;
		}
	}

	fstype = file_system_type_get(type);
	if (NULL == fstype) {
		DPRINT("fail to find file system type %s!\n", type);
		return -ENOENT;
	}

	root = fstype->mount(fstype, flags, bdev_name);
	if (!root) {
		DPRINT("fail to mount %s!\n", bdev_name);
		ret = -EIO; // fixme!
		goto L1;
	}

	mnt = zalloc(sizeof(*mnt));
	if (NULL == mnt) {
		ret = -ENOMEM;
		goto L2;
	}

	mnt->fstype   = fstype;
	mnt->dev_name = bdev_name;
	mnt->root     = root;

	if (flags & MS_ROOT) {
		mnt->mountpoint = NULL;
		set_fs_root(mnt, root);
		set_fs_pwd(mnt, root); // now?
	} else {
		mnt->mountpoint = nd.dentry;
	}

	list_add_tail(&mnt->mnt_hash, &g_mount_list);

	return 0;
L2:
	//
L1:
	return ret;
}

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
		// fixme
		if (!strncmp(mnt->mountpoint->d_name.name, unit->name, unit->len))
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

struct dentry *d_lookup(struct dentry *parent, struct qstr *unit)
{
	struct dentry *de;
	struct list_node *iter;

	list_for_each(iter, &parent->d_subdirs) {
		de = container_of(iter, struct dentry, d_child);
		if (!strcmp(de->d_name.name, unit->name))
			return de;
	}

	return NULL;
}

static struct dentry *real_lookup(struct dentry *parent, struct qstr *unit,
		     struct nameidata *nd)
{
	struct inode *dir = parent->d_inode;
	struct dentry *dentry = d_alloc(parent, unit);

	if (dentry) {
		struct dentry *result;

		assert(dir->i_op->lookup);
		result = dir->i_op->lookup(dir, dentry, nd);
		// fixme: return dentry instead of NULL;
		if (result) {
			dput(dentry);
			dentry = result;
		}
	}

	return dentry;
}

static int __follow_mount(struct path *path)
{
	return 0;
}

static int do_lookup(struct nameidata *nd, struct qstr *name,
		     struct path *path)
{
	struct vfsmount *mnt = nd->mnt;
	struct dentry *dentry;

	dentry = d_lookup(nd->dentry, name);
	if (!dentry) {
		dentry = real_lookup(nd->dentry, name, nd);
		if (!dentry)
			return -ENOENT;
	}

	path->mnt = mnt;
	path->dentry = dentry;

	__follow_mount(path);

	return 0;
}

int path_walk(const char *path, struct nameidata *nd)
{
	int ret;
	struct qstr unit;
	struct path next;

#if 0
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
#endif

	if ('/' == *path) {
		nd->dentry = get_curr_fs()->root;
		nd->mnt = get_curr_fs()->rootmnt;
	} else {
		nd->dentry = get_curr_fs()->pwd;
		nd->mnt = get_curr_fs()->pwdmnt;
	}

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

		ret = do_lookup(nd, &unit, &next);
		if (ret < 0)
			return ret;

		nd->dentry = next.dentry;
		nd->mnt = next.mnt;
	}

	return 0;
}

static int __dentry_open(struct dentry *dir, struct file *fp)
{
	int ret;
	struct inode *inode = dir->d_inode;

	fp->f_dentry = dir;
	fp->f_pos = 0;

	fp->f_op = inode->i_fop;
	if (!fp->f_op) {
		GEN_DBG("No fops for %s!\n", dir->d_name.name);
		return -ENODEV;
	}

	// TODO: check flags

	if (fp->f_op->open) {
		ret = fp->f_op->open(fp, inode);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int sys_open(const char *path, int flags, int mode)
{
	int fd, ret;
	struct file *fp;
	struct nameidata nd;

	fd = get_unused_fd();
	if (fd < 0)
		return fd;

	ret = path_walk(path, &nd);
	if (ret < 0) {
		GEN_DBG("fail to find \"%s\"!\n", path);
		return ret;
	}

	fp = zalloc(sizeof(*fp)); // use malloc instead of zalloc
	if (!fp) {
		ret = -ENOMEM;
		goto no_mem;
	}

	fp->flags = flags;

	ret = __dentry_open(nd.dentry, fp);
	if (ret < 0)
		goto fail;

	fd_install(fd, fp);

	return fd;

fail:
	free(fp);
no_mem:
	return ret;
}

int GAPI open(const char *path, int flags, ...)
{
	int mode = 0;

	return sys_open(path, flags, mode);
}

EXPORT_SYMBOL(open);

int GAPI close(int fd)
{
	struct file *fp;

	fp = fget(fd);
	if (!fp)
		return -ENOENT;

	fd_install(fd, NULL);

	if (fp->f_op && fp->f_op->close)
		return fp->f_op->close(fp);

	return 0;
}

EXPORT_SYMBOL(close);

ssize_t GAPI read(int fd, void *buff, size_t size)
{
	struct file *fp;

	fp = fget(fd);
	if (!fp || !fp->f_op->read)
		return -ENOENT;

	return fp->f_op->read(fp, buff, size, &fp->f_pos);
}

EXPORT_SYMBOL(read);

ssize_t GAPI write(int fd, const void *buff, size_t size)
{
	struct file *fp;

	fp = fget(fd);
	if (!fp || !fp->f_op->write)
		return -ENOENT;

	return fp->f_op->write(fp, buff, size, &fp->f_pos);
}

EXPORT_SYMBOL(write);

int sys_ioctl(int fd, int cmd, unsigned long arg)
{
	struct file *fp;

	fp = fget(fd);
	if (!fp || !fp->f_op->ioctl)
		return -ENOENT;

	return fp->f_op->ioctl(fp, cmd, arg);
}

int GAPI ioctl(int fd, int cmd, ...)
{
	unsigned long arg;

	arg = *((unsigned long *)&cmd + 1); // fixme

	return sys_ioctl(fd, cmd, arg);
}

EXPORT_SYMBOL(ioctl);

loff_t GAPI lseek(int fd, loff_t offset, int whence)
{
	struct file *fp;

	fp = fget(fd);
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
