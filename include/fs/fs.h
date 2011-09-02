#pragma once

#include <types.h>
#include <block.h>

#define O_RDONLY 0
#define O_RDWD   1

#define MAX_FILE_NAME_LEN 64

struct file_system_type
{
	const char *name;
	int (*mount)(struct file_system_type *, unsigned long, struct block_device *);	
	struct file_system_type *next;
};

int file_system_type_register(struct file_system_type *);

struct file_system_type *file_system_type_get(const char *);

int mount(const char *type, unsigned long flags, const char *bdev_name, const char *path);
