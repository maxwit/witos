#include <platform.h>

static struct bus platform_bus;

static int match(struct device *dev, struct driver *drv)
{
	struct platform_device *plat_dev;
	// struct platform_driver *plat_drv;

	plat_dev = container_of(dev, struct platform_device, dev);
	// plat_drv = container_of(drv, struct platform_driver, drv);

	// printf("%s(): 0x%08p.%s == %s\n", __func__, plat_dev, plat_dev->name, drv->name);
	return !strcmp(plat_dev->name, drv->name);
}

static int platform_driver_init(struct device *dev)
{
	struct platform_device *plat_dev;
	struct platform_driver *plat_drv;

	plat_dev = container_of(dev, struct platform_device, dev);
	plat_drv = container_of(dev->drv, struct platform_driver, drv);

	return plat_drv->init(plat_dev);
}

int platform_device_register(struct platform_device *plat_dev)
{
	plat_dev->dev.bus = &platform_bus;

	// printf("%s(): 0x%08p.%s\n", __func__, plat_dev, plat_dev->name);

	return device_register(&plat_dev->dev);
}

int platform_driver_register(struct platform_driver *plat_drv)
{
	plat_drv->drv.bus = &platform_bus;
	plat_drv->drv.init = platform_driver_init;

	return driver_register(&plat_drv->drv);
}

static int __INIT__ platform_init(void)
{
	platform_bus.match = match;

	list_head_init(&platform_bus.dev_list);
	list_head_init(&platform_bus.drv_list);

	return 0;
}

SUBSYS_INIT(platform_init);
