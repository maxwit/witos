#include <block.h>  // fixme

#define FD_MAX_SIZE    256 

static struct bdev_file *fd_array[FD_MAX_SIZE];

int bdev_open(const char *name, int flags, ...)
{
	int i, ret;
	struct bdev_file *fp;
	struct block_device *bdev;

	bdev = get_bdev_by_name(name);
	if (!bdev)
		return -ENODEV;

	fp = zalloc(sizeof(*fp));
	if (!fp) {
		ret = -ENOMEM;
		goto no_mem;
	}

	fp->bdev = bdev;
	fp->cur_pos = 0;
	fp->fops = bdev->fops;

	if (!fp->fops) {
		GEN_DGB("No fops for %s!\n", name);
		ret = -EINVAL;
		goto fail;
	}

	if (fp->fops->open) {
		ret = fp->fops->open(fp, flags);
		if (ret < 0)
			goto fail;
	}

	for (i = 0; i < FD_MAX_SIZE; i++) {
		if (!fd_array[i]) {
			fd_array[i] = fp;
			return i;
		}
	}

	// TODO: close file when failed

fail:
	free(fp);
no_mem:
	return -EBUSY;
}

int bdev_close(int fd)
{
	struct bdev_file *fp;

	if (fd < 0 || fd >= FD_MAX_SIZE)
		return -EINVAL;

	fp = fd_array[fd];
	if (!fp)
		return 0;

	if (fp->fops && fp->fops->close)
		return fp->fops->close(fp);

	return 0;
}

ssize_t bdev_read(int fd, void *buff, size_t size)
{
	struct bdev_file *fp;

	if (fd < 0 || fd >= FD_MAX_SIZE)
		return -EINVAL;

	fp = fd_array[fd];
	if (!fp)
		return -ENODEV;

	if (fp->fops && fp->fops->read)
		return fp->fops->read(fp, buff, size, &fp->cur_pos);

	return -ENODEV;
}

ssize_t bdev_write(int fd, const void * buff, size_t size)
{
	struct bdev_file *fp;

	if (fd < 0 || fd >= FD_MAX_SIZE)
		return -EINVAL;

	fp = fd_array[fd];
	if (!fp)
		return -ENODEV;

	if (fp->fops && fp->fops->write)
		return fp->fops->write(fp, buff, size, &fp->cur_pos);

	return -ENODEV;
}

int bdev_ioctl(int fd, int cmd, ...)
{
	struct bdev_file *fp;

	if (fd < 0 || fd >= FD_MAX_SIZE)
		return -EINVAL;

	fp = fd_array[fd];
	if (!fp)
		return -ENODEV;

	if (fp->fops && fp->fops->ioctl) {
		// fixme
		unsigned long arg, *sp;

		sp = (unsigned long *)&cmd;
		arg = sp[1];

		return fp->fops->ioctl(fp, cmd, arg);
	}

	return -ENODEV;
}
