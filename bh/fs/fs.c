#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
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

#ifdef __G_BIOS__
int mount(const char *type, unsigned long flags, const char *bdev_name, const char *path)
#else
int gb_mount(const char *type, unsigned long flags, const char *bdev_name, const char *path)
#endif
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

#ifdef __G_BIOS__
int umount(const char *mnt)
#else
int gb_umount(const char *mnt)
#endif
{
	return 0;
}

#ifdef __G_BIOS__
int open(const char *const name, int flags, ...)
#else
int gb_open(const char *const name, int flags, ...)
#endif
{
	int fd;
	const char *fn;
	struct file *fp;
	struct mount_point *mnt;
	const struct file_operations *fops;

	for (fn = name; *fn != ':' && *fn != '\0'; fn++);

	for (mnt = g_mnt_list; mnt; mnt = mnt->next) {
		if (!strncmp(mnt->path, name, fn - name))
			break;
	}

	if (NULL == mnt)
		return -ENOENT;

	fops = mnt->fs_type->fops;

	if (*fn == '\0') {
		DPRINT("%s(): Invalid file name \"%s\"!\n", __func__, name);
		return -EINVAL;
	}
	fn++;

	fp = fops->open(mnt->bdev->fs, fn, flags, 0);
	if (NULL == fp)
		return -ENOENT;

	for (fd = 0; fd < MAX_FDS; fd++)
		if (!fd_array[fd])
			break;

	if (fd == MAX_FDS)
		return -EBUSY;

	// fp->mode = ...
	fp->pos = 0;
	fp->fops = fops;

	fd_array[fd] = fp;

	return fd;
}

#ifdef __G_BIOS__
int close(int fd)
#else
int gb_close(int fd)
#endif
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp)
		return -ENOENT;

	return fp->fops->close(fp);
}

#ifdef __G_BIOS__
int read(int fd, void *buff, size_t size)
#else
int gb_read(int fd, void *buff, size_t size)
#endif
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp)
		return -ENOENT;

	return fp->fops->read(fp, buff, size);
}

#ifdef __G_BIOS__
int write(int fd, const void *buff, size_t size)
#else
int gb_write(int fd, const void *buff, size_t size)
#endif
{
	struct file *fp;

	if (fd < 0 || fd >= MAX_FDS)
		return -EINVAL;

	fp = fd_array[fd];

	if (!fp)
		return -ENOENT;

	return fp->fops->write(fp, buff, size);
}
