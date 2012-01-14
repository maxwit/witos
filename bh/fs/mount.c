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
		list_for_each_entry(mnt, &g_mount_list, mnt_hash) {
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
		struct path sysroot = {.dentry = root, .mnt = mnt};

		mnt->mountpoint = NULL; // fixme
		mnt->mnt_parent = NULL;
		set_fs_root(&sysroot);
		set_fs_pwd(&sysroot); // now?
	} else {
		mnt->mountpoint = nd.path.dentry;
		mnt->mnt_parent = nd.path.mnt;
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

static inline void path_to_nameidata(const struct path *path,
					struct nameidata *nd)
{
	nd->path.mnt = path->mnt;
	nd->path.dentry = path->dentry;
}

struct dentry *d_lookup(struct dentry *parent, struct qstr *unit)
{
	struct dentry *de;
	struct list_node *iter;

	list_for_each(iter, &parent->d_subdirs) {
		de = container_of(iter, struct dentry, d_child);
		if (de->d_name.len == unit->len && \
			!strncmp(de->d_name.name, unit->name, unit->len))
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

		result = dir->i_op->lookup(dir, dentry, nd);
		if (nd->ret < 0) {
			GEN_DBG("fail to lookup \"%s\" (ret = %d)!\n",
				dentry->d_name.name, nd->ret);
			return NULL;
		}
		if (result != dentry) {
			GEN_DBG("%s -> %s\n", dentry->d_name.name, result->d_name.name);
			dput(dentry);
			dentry = result;
		}
	}

	return dentry;
}


struct vfsmount *lookup_mnt(struct path *path)
{
	struct vfsmount *mnt;

	list_for_each_entry(mnt, &g_mount_list, mnt_hash) {
		if (mnt->mountpoint == path->dentry)
			return mnt;
	}

	return NULL;
}

int __follow_mount(struct path *path)
{
	struct vfsmount *mnt;

	mnt = lookup_mnt(path);
	if (mnt) {
		path->mnt = mnt;
		path->dentry = mnt->root;
	}

	return 0;
}

int follow_up(struct path *path)
{
	path->dentry = path->mnt->mountpoint;
	path->mnt = path->mnt->mnt_parent;

	return 0;
}

static int do_lookup(struct nameidata *nd, struct qstr *name,
		     struct path *path)
{
	struct vfsmount *mnt = nd->path.mnt;
	struct dentry *dentry;

	if ('.' == name->name[0]) {
		if (1 == name->len)
			return 0;

		if ('.' == name->name[1] && 2 == name->len) {
			path->dentry = nd->path.dentry->d_parent;
			path->mnt = nd->path.mnt;

			if (path->dentry == path->mnt->root && path->mnt->mnt_parent)
				follow_up(path);

			return 0;
		}
	}

	dentry = d_lookup(nd->path.dentry, name);
	if (!dentry) {
		dentry = real_lookup(nd->path.dentry, name, nd);
		if (!dentry)
			return nd->ret;
	}

	path->mnt = mnt;
	path->dentry = dentry;

	__follow_mount(path);

	return 0;
}

static inline bool cannot_lookup(const struct inode *in)
{
	return in->i_op->lookup == NULL;
}

int path_walk(const char *path, struct nameidata *nd)
{
	int ret;
	struct qstr unit;
	struct path next;

	if ('/' == *path) {
		get_fs_root(&nd->path);
	} else {
		get_fs_pwd(&nd->path);
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

		if (cannot_lookup(nd->path.dentry->d_inode)) {
			GEN_DBG("dentry \"%s\" cannot lookup! i_op = %p\n",
				nd->path.dentry->d_name.name, nd->path.dentry->d_inode->i_op);
			return -ENOTDIR;
		}

		GEN_DBG("searching \"%s\"\n", path);
		ret = do_lookup(nd, &unit, &next);
		if (ret < 0)
			return ret;

		path_to_nameidata(&next, nd);
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
		if (ret < 0) {
			GEN_DBG("open failed! (ret = %d)\n", ret);
			return ret;
		}
	}

	return 0;
}

static int sys_open(const char *path, int flags, int mode)
{
	int fd, ret;
	struct file *fp;
	struct nameidata nd = {.ret = 0};

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

	ret = __dentry_open(nd.path.dentry, fp);
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

ssize_t GAPI read(int fd, void *buff, size_t size)
{
	struct file *fp;

	fp = fget(fd);
	if (!fp || !fp->f_op->read)
		return -ENOENT;

	return fp->f_op->read(fp, buff, size, &fp->f_pos);
}

ssize_t GAPI write(int fd, const void *buff, size_t size)
{
	struct file *fp;

	fp = fget(fd);
	if (!fp || !fp->f_op->write)
		return -ENOENT;

	return fp->f_op->write(fp, buff, size, &fp->f_pos);
}

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
