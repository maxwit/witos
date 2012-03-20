#include <init.h>
#include <string.h>
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
	int ret;

	plat_dev->dev.bus = &platform_bus;

	if (plat_dev->init) {
		ret = plat_dev->init(plat_dev);
		if (ret < 0)
			return ret;
	}

	return device_register(&plat_dev->dev);
}

int platform_driver_register(struct platform_driver *plat_drv)
{
	plat_drv->drv.bus = &platform_bus;
	plat_drv->drv.init = platform_driver_init;
	list_head_init(&plat_drv->drv.dev_list);

	return driver_register(&plat_drv->drv);
}

static int __INIT__ platform_init(void)
{
	return 0;
}

SUBSYS_INIT(platform_init);
