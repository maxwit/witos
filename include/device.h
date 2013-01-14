#pragma once

#include <list.h>

#define MAX_DEV_NAME 64

#define MINORBITS	20
#define MINORMASK	((1U << MINORBITS) - 1)

#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))

#define IORESOURCE_MEM 0x1
#define IORESOURCE_IRQ 0x2

struct device;
struct driver;
struct bus_type;

struct resource {
	unsigned long start;
	size_t size;
	unsigned int flag;
};

struct device {
	struct resource *resources;
	int res_num;

	struct bus_type *bus;
	struct list_head bus_node;

	struct driver *drv;
	struct list_head drv_node;
};

struct driver {
	const char *name;

	struct bus_type *bus;
	struct list_head bus_node;

	struct list_head dev_list;

	int (*probe)(struct device *);
};

// PCI, USB, AHB/APB (platform), SPI, IIC
struct bus_type {
	const char *name;
	int (*match)(struct device *, struct driver *);
	struct list_head dev_list, drv_list;
};

int device_register(struct device *dev);
int driver_register(struct driver *drv);
