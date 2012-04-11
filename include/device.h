#pragma once

#include <list.h>

#define MAX_DEV_NAME 64

#define MINORBITS	20
#define MINORMASK	((1U << MINORBITS) - 1)

#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))

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
	struct list_head bus_node;

	struct driver *drv;
	struct list_head drv_node;
};

struct driver {
	const char *name;

	struct bus *bus;
	struct list_head bus_node;

	struct list_head dev_list;

	int (*init)(struct device *);
};

struct bus {
	const char *name;
	int (*match)(struct device *, struct driver *);
	struct list_head dev_list, drv_list;
};

int device_register(struct device *dev);
int driver_register(struct driver *drv);
