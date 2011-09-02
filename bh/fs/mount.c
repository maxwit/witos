#include <fs/fs.h>

int mount(const char *type, unsigned long flags, const char *bdev_name, const char *path)
{
	struct block_device *bdev;
	struct file_system_type *fs_type;

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

	return fs_type->mount(fs_type, flags, bdev);
}

static struct file_system_type *fs_type_list = NULL;

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
