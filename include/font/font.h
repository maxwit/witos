#pragma once

#include <init.h>
#include <types.h>

typedef int (*font_init_t)(void);

#define __GBIOS_FONT_LIST__ __attribute__((section(".Level0.gbios_font")))
#define __GBIOS_FONT__ __attribute__((section(".Level1.gbios_font")))

#define FONT_LIST_INIT(font_list_init) \
	static const __USED__ __GBIOS_FONT_LIST__ font_init_t __gbios_font_##font_list_init = font_list_init;

#define INSTALL_FONT(font_init) \
	static const __USED__ __GBIOS_FONT__ font_init_t ___gbios_font_##font_init = font_init;

struct font_descript {
    const char *name;
    int width, height;
    const void *data;
};

struct font_list_node {
	struct font_descript *font;
	struct list_node node;
};

extern void add_font(struct font_descript *);
extern void show_all_fonts();
extern struct font_descript *find_font(const char *name);
