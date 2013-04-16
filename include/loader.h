#pragma once

#include <types.h>
#include <init.h>

struct loader_opt {
	void *load_addr; // fixme: void *load_addr[2];
	int  load_flash; // fixme
	int  load_size;
	const char *prompt;
	char file_name[FILE_NAME_SIZE];
	const char *dst;
};
