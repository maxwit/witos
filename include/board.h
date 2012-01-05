#pragma once

#include <types.h>
#include <init.h>

#define __BOARD__  __attribute__((__section__(".gsect.board")))

#define BOARD_ID(id1, id2) {.name = id1, .mach_id = id2}

struct board_id {
	const char *name;
	int mach_id;
};

struct board_desc {
	const char *name;
	const struct board_id *id_table;
	int (*init)(struct board_desc *, const struct board_id *);
};

#define BOARD_DESC(name1, idt, init1) \
	static struct board_desc __BOARD__ __USED__ __board_##__LINE__ = { \
		.name = name1, \
		.id_table = idt, \
		.init = init1, \
	}

int __INIT__ board_init(void);

const struct board_id *board_get_active();
