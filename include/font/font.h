#pragma once

#include <init.h>
#include <types.h>

#define FONT_LIST_INIT SUBSYS_INIT
#define FONT_INIT DRIVER_INIT

struct font_descript
{
    const char *name;
    int width, height;
    const void *data;
};

struct font_list_node
{
	struct font_descript *font;
	struct list_node node;
};

extern void add_font(struct font_descript *);
extern void show_all_fonts();
extern struct font_descript *find_font(const char *name);
