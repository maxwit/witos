#pragma once
#include <types.h>

#define __GBIOS_FONT__ __USED__ __attribute__((section(".gbios_font")))

struct font_desc {
    const char *name;
    int width, height;
    const void *data;
};

const struct font_desc *find_font(const char *name);
