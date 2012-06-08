#pragma once
#include <list.h>
#include <block.h>

struct qdev_node {
	struct list_head dev_node;
	void *data;
};

int device_enqueue(struct block_device *bdev);
int device_monitor();
