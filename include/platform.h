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

	int (*init)(struct platform_device *);
};

// fixme
static inline unsigned long platform_get_memory(struct platform_device *plat_dev)
{
	return plat_dev->dev.mem;
}

static inline unsigned long platform_get_irq(struct platform_device *plat_dev)
{
	return plat_dev->dev.irq;
}

int platform_device_register(struct platform_device *dev);
int platform_driver_register(struct platform_driver *drv);
