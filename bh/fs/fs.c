#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <fs/fs.h>

#define MAX_FDS 256

static struct file_system_type *fs_type_list;
static struct mount_point *g_mnt_list;
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

int mount(const char *type, unsigned long flags, const char *bdev_name, const char *path)
{
	int ret;
	struct block_device *bdev;
	struct file_system_type *fs_type;
	struct mount_point *mnt;

	bdev = get_bdev_by_name(bdev_name);
	if (NULL == bdev) {
		DPRINT("fail to open block device \"%s\"!\n", bdev_name);
		return -ENODEV;
	}

	fs_type = file_system_type_get(type);
	if (NULL == fs_type)
		return -ENOENT;
	printf("%s() line %d\n", __func__, __LINE__);

	ret = fs_type->mount(fs_type, flags, bdev);
	if (ret < 0) {
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

	// TODO: check if mp exists or not!
#if 0
	for (mp = &g_mnt_list; *mp; mp = &(*mp)->next);
	*mp = mnt;
#else
	mnt->next = g_mnt_list;
	g_mnt_list = mnt;
#endif

	return 0;
L2:
	//
L1:
	return ret;
}

int umount(const char *mnt)
{
	return 0;
}

static int regular_open(struct file *fp, const char *path)
{
	const char *fn;
	struct mount_point *mnt;
	// const struct file_operations *fops;
	struct file_system_type *fs_type;

	for (fn = path; *fn != ':' && *fn != '\0'; fn++);

	for (mnt = g_mnt_list; mnt; mnt = mnt->next) {
		if (!strncmp(mnt->path, path, fn - path))
			break;
	}

	if (NULL == mnt)
		return -ENOENT;

	fs_type = mnt->fs_type;

	if (!fs_type->lookup || !fs_type->fops)
		return -EINVAL;

	fp->fops = fs_type->fops;

	if (*fn == '\0') {
		DPRINT("%s(): Invalid file path \"%s\"!\n", __func__, path);
		return -EINVAL;
	}
	fn++;

	fp->de = fs_type->lookup(mnt->bdev->fs, fn);
	if (!fp->de)
		return -ENOENT;

	fp->mnt = mnt;

	return 0;
}

static int bdev_open(struct file *fp, const char *name)
{
	struct block_device *bdev;

	bdev = get_bdev_by_name(name);
	if (!bdev)
		return -ENODEV;

	fp->bdev = bdev;
	fp->fops = bdev->fops;

	return 0;
}

static inline bool check_bdev_file(const char *path)
{
	// fixme!
	return !strncmp(path, "mtdblock", 8) || \
		!strncmp(path, "mmcblk", 6);
}

int open(const char *path, int flags, ...)
{
	int fd, ret;
	int mode = 0; //  fixme
	struct file *fp;

	fp = zalloc(sizeof(*fp));
	if (!fp) {
		ret = -ENOMEM;
		goto no_mem;
	}

	// fp->cur_pos = 0;

	if (check_bdev_file(path))
		ret = bdev_open(fp, path);
	else
		ret = regular_open(fp, path);

	if (ret < 0)
		goto fail;

	if (!fp->fops) {
		GEN_DGB("No fops for %s!\n", path);
		goto fail;
	}

	if (fp->fops->open) {
		ret = fp->fops->open(fp, flags, mode);
		if (ret < 0)
			goto fail;
	}

	for (fd = 0; fd < MAX_FDS; fd++) {
		if (!fd_array[fd]) {
			fd_array[fd] = fp;
			return fd;
		}
	}

	// TODO: close file when failed

fail:
	free(fp);
no_mem:
	return ret;
}

int close(int fd)
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

int read(int fd, void *buff, size_t size)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->fops->read)
		return -ENOENT;

	return fp->fops->read(fp, buff, size, &fp->pos);
}

int write(int fd, const void *buff, size_t size)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp || !fp->fops->write)
		return -ENOENT;

	return fp->fops->write(fp, buff, size, &fp->pos);
}

int ioctl(int fd, int cmd, ...)
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

loff_t lseek(int fd, loff_t offset, int whence)
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp)
		return -ENOENT;

#if 0
	if (fp->fops->lseek)
		return fp->fops->lseek(fp, offset, whence);
#endif

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
