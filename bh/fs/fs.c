#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <fs/fs.h>

#define MAX_FDS 256

static struct file *fd_array[MAX_FDS];

struct super_block *sget(struct file_system_type *type, struct block_device *bdev)
{
	struct super_block *sb;

	sb = zalloc(sizeof(*sb));
	if (!sb) {
		// ...
		return NULL;
	}

	sb->s_bdev = bdev;

	return sb;
}

struct dentry *__d_alloc(struct super_block *sb, const struct qstr *str)
{
	char *name;
	struct dentry *de;

	de = zalloc(sizeof(*de));
	if (!de)
		return NULL;

	de->d_sb = sb;

	de->d_name.len = str->len;
	if (str->len >= DNAME_INLINE_LEN) {
		name = malloc(str->len + 1);
		if (!name) {
			free(de);
			return NULL;
		}
	} else {
		name = de->d_iname;
	}
	de->d_name.name = name;
	strncpy(name, str->name, str->len);
	name[str->len] = '\0';

	return de;
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

struct file *fget(unsigned int fd)
{
	if (fd < 0 || fd >= MAX_FDS)
		return NULL; // -EINVAL;

	return fd_array[fd];
}
