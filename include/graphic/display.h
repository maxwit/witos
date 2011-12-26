#pragma once

#include <types.h>

typedef enum {
	PIX_RGB15,
	PIX_RGB16,
	PIX_RGB24,
	PIX_RGB32,
} pixel_format_t;

struct lcd_vmode {
	const char *model;
	int pix_clk;
	int width, height;
	int hfp, hbp, hpw;
	int vfp, vbp, vpw;
};

#if 0
struct plat_lcd_info {
	const char *model;
	pixel_format_t pix_format;
};
#endif

const struct lcd_vmode *lcd_get_vmode_by_id(int lcd_id);
const struct lcd_vmode *lcd_get_vmode_by_name(const char *model);
void *video_mem_alloc(unsigned long *phy_addr, const struct lcd_vmode *vm, pixel_format_t pix_fmt);

//
struct display {
	void *mmio;
	void *video_mem_va;
	__u32 video_mem_pa;

	pixel_format_t pix_fmt;
	struct lcd_vmode *video_mode;

	int (*set_vmode)(struct display *, const struct lcd_vmode *);
};

struct display* display_create(void);
int display_register(struct display* disp);
struct display* get_system_display(void);
