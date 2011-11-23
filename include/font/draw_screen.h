#pragma once

#include <types.h>

int set_screen(__u32 pixel);

int draw_char(struct display *disp, int x, int y, struct font_desc *font, __u32 color, char c);
int draw_string(struct display *disp, int x, int y, struct font_desc *font, __u32 color, const char * str);
