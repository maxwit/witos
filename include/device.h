#pragma once

#include <list.h>

#define MAX_DEV_NAME 64

struct device;
struct driver;
struct bus;

struct resource {
	unsigned long stat;
	size_t size;
};

struct device {
	unsigned long mem;
	// size_t size;
	int irq;

	struct bus *bus;
	struct list_node bus_node;

	struct driver *drv;
	struct list_node drv_node;
};

struct driver {
	const char *name;

	struct bus *bus;
	struct list_node bus_node;

	struct list_node dev_list;

	int (*init)(struct device *);
};

struct bus {
	const char *name;
	int (*match)(struct device *, struct driver *);
	struct list_node dev_list, drv_list;
};

int device_register(struct device *dev);
int driver_register(struct driver *drv);
