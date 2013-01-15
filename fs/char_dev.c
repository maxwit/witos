#include <fs.h>

#define MAX_CDEV 256

struct cdev {
	int minor;
	char *name;
	struct cdev *next;
};

struct cdev_xx {
	int major;
	struct file_operations *fops;
	struct cdev *cdev_list;
};

static struct cdev_xx *g_cdev_xx[MAX_CDEV];

int register_chrdev(int major, const struct file_operations *fops, const char *name)
{
	int i;
	struct cdev_xx *xx;

	if (!major) {
		for (i = MAX_CDEV - 1; i > 0; i--)
			if (!g_cdev_xx[i])
				break;

		major = i;
	}

	if (major <= 0 || major >= MAX_CDEV || g_cdev_xx[major])
		return -EBUSY;

	// TODO
	xx = zalloc(sizeof(*xx));
	// if

	xx->fops = fops;
	xx->major = major;

	g_cdev_xx[major] = xx;

	return major;
}

int device_create(dev_t devt, const char *fmt, ...)
{
	int major = MAJOR(devt);
	struct cdev *cdev, **p;

	cdev = zalloc(sizeof(*cdev));
	// ..

	cdev->minor = MINOR(devt);
	cdev->name = fmt; // fixme
	cdev->next = NULL;

	for (p = &g_cdev_xx[major]->cdev_list; *p; p = &(*p)->next)
		if ((*p)->minor == cdev->minor)
			return -EBUSY;

	*p = cdev;

	return 0;
}
