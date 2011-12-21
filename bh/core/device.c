#include <device.h>
#include <list.h>

static struct bus general_bus;

static int match(struct device *dev, struct driver *drv)
{
	return !strcmp(dev->name, drv->name);
}

int device_register(struct device *dev)
{
	int ret;
	struct list_node *iter;
	struct driver *drv;

	// step 1:
	list_add_tail(&dev->bus_node, &general_bus.dev_list);

	// step 2:
	list_for_each(iter, &general_bus.drv_list) {
		drv = container_of(iter, struct driver, bus_node);
		if (match(dev, drv)) {
			dev->drv = drv;
			list_add_tail(&dev->drv_node, &drv->dev_list);

			// step 3:
			ret = drv->init(dev);
			if (!ret)
				return 0;
		}
	}

	return 0;
}

int driver_register(struct driver *drv)
{
	int ret;
	struct list_node *iter;
	struct device *dev;

	list_add_tail(&drv->bus_node, &general_bus.drv_list);

	list_for_each(iter, &general_bus.dev_list) {
		dev = container_of(iter, struct device, bus_node);
		if (match(dev, drv)) {
			dev->drv = drv;
			list_add_tail(&dev->drv_node, &drv->dev_list);

			ret = drv->init(dev);
			if (ret < 0)
				dev->drv = NULL;
		}
	}

	return 0;
}

static int __INIT__ bus_init(void)
{
	list_head_init(&general_bus.dev_list);
	list_head_init(&general_bus.drv_list);

	return 0;
}

SUBSYS_INIT(bus_init);
