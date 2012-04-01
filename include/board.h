#pragma once

#include <types.h>
#include <init.h>

#define __BOARD__  __attribute__((__section__(".gsect.board")))

struct board_desc {
	const char *name;
	int mach_id; // machine ID for Linux
	int (*init)(struct board_desc *);
};

#define BOARD_DESC(name1, idt, init1) \
	static struct board_desc __BOARD__ __USED__ __board_##__LINE__ = { \
		.name = name1, \
		.mach_id = idt, \
		.init = init1, \
	}

int __init board_init(void);

const struct board_desc *board_get_active();
