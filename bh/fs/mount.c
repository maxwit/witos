#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fs/fs.h>

static struct file_system_type *fs_type_list = NULL;
static struct mount_point *g_mnt_list = NULL;

int file_system_type_register(struct file_system_type *fs_type)
{
	struct file_system_type **p;

	for (p = &fs_type_list; *p; p = &(*p)->next)
	{
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

	for (p = fs_type_list; p; p = p->next)
	{
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
	struct mount_point *mnt; // , **mp;

	bdev = get_bdev_by_name(bdev_name);
	if (NULL == bdev)
	{
		DPRINT("fail to open block device \"%s\"!\n", bdev_name);
		return -ENODEV;
	}

	fs_type = file_system_type_get(type);
	if (NULL == fs_type)
	{
		return -ENOENT;
	}
	printf("%s() line %d\n", __func__, __LINE__);

	ret = fs_type->mount(fs_type, flags, bdev);
	if (ret < 0)
	{
		DPRINT("fail to mount %s!\n", bdev_name);
		goto L1;
	}

	mnt = zalloc(sizeof(*mnt));
	if (NULL == mnt)
	{
		ret = -ENOMEM;
		goto L2;
	}

	mnt->fs_type = fs_type;
	mnt->path = path;

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
