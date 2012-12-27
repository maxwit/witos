#include <errno.h>
#include <device.h>

struct platform_id {
	const char *name;
};

struct platform_device {
	struct device dev;
	const char *name;
	int (*init)(struct platform_device *); // to be removed!
};

struct platform_driver {
	struct driver drv;
	// struct platform_id *idt;

	int (*probe)(struct platform_device *);
};

static inline struct resource *platform_get_mem(struct platform_device *pdev, int n)
{
	int i, j;

	for (i = 0, j = 0; i < pdev->dev.res_num; i++)
		if (pdev->dev.resources[i].flag & IORESOURCE_MEM) {
			if (j == n)
				return pdev->dev.resources + i;
			j++;
		}

	return NULL;
}

static inline int platform_get_irq(struct platform_device *pdev, int n)
{
	int i, j;

	for (i = 0, j = 0; i < pdev->dev.res_num; i++)
		if (pdev->dev.resources[i].flag & IORESOURCE_IRQ) {
			if (j == n)
				return pdev->dev.resources[i].start;
			j++;
		}

	return -ENOENT;
}

int platform_device_register(struct platform_device *dev);
int platform_driver_register(struct platform_driver *drv);
