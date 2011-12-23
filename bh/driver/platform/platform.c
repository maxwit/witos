#include <platform.h>

static int match(struct device *dev, struct driver *drv);

static struct bus platform_bus = {
	.name     = "platform",
	.match    = match,
	.dev_list = HEAD_INIT(platform_bus.dev_list),
	.drv_list = HEAD_INIT(platform_bus.drv_list),
};

static int match(struct device *dev, struct driver *drv)
{
	struct platform_device *plat_dev;
	// struct platform_driver *plat_drv;

	plat_dev = container_of(dev, struct platform_device, dev);
	// plat_drv = container_of(drv, struct platform_driver, drv);

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
	return 0;
}

SUBSYS_INIT(platform_init);
