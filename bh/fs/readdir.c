#include <errno.h>
#include <string.h>
#include <fs/fs.h>

int filldir(struct linux_dirent *lde, const char *name, int size, loff_t offset,
		   unsigned long ino, unsigned int type)
{
	size_t reclen;

	lde->d_ino  = ino;
	lde->d_off  = offset;
	lde->d_type = type;

	reclen = (size_t)(&((struct linux_dirent *)0)->d_name) + size + 2;
	ALIGN_UP(reclen, sizeof(long));
	lde->d_reclen = reclen;

	strcpy(lde->d_name, name);

	return 0;
}

int sys_getdents(unsigned int fd, struct linux_dirent *lde, unsigned int count)
{
	int ret;
	struct file *fp;

	fp = fget(fd);
	if (!fp || !fp->f_op)
		return -ENODEV;

	if (!fp->f_op->readdir)
		return -ENOTSUPP;

	ret = fp->f_op->readdir(fp, lde);
	return ret;
}
