#include <fs.h>

struct cdev {
	int major, minor;
	struct file_operations *fops;
	char *name;
	struct cdev *next;
};

int chrdev_register(int major, const struct file_operations *fops, const char *name)
{
	struct cdev *cdev;

	cdev = zalloc(sizeof(*cdev));
	// ...
	cdev->fops = fops;
	//...

	// list_add

	return 0;
}

struct device *device_create(dev_t devt, const char *fmt, ...)
{
	return NULL;
}
