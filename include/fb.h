#pragma once

#include <types.h>
#include <fs.h>

struct fb_var_screeninfo {
	int xres, yres;
	
};

struct fb_fix_screeninfo {
	unsigned long smem_start;
	size_t smem_len;
};


struct fb_ops {
	int (*fb_check_var)(struct fb_info *, struct fb_var_screeninfo *);
	int (*fb_set_par)(struct fb_info *);
};

struct fb_info {
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;
	struct fb_ops *ops;
	char *screenbase;
	void *par;
};

struct fb_info *framebuffer_alloc(size_t);
// framebuffer_free()

int framebuffer_register(struct fb_info *);
// framebuffer_unregister

