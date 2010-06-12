#pragma once

#include <types.h>

typedef enum
{
	PIX_RGB15,
	PIX_RGB16,
	PIX_RGB24,
} EPixFormat;


struct lcd_vmode
{
	const char *name;
	int pixclk;
	int width, height;
	int hfp, hbp, hpw;
	int vfp, vbp, vpw;
};

void *video_mem_alloc(u32 uWidth, u32 uHeight, EPixFormat ePixFormat, u32 *ulPhyAddr);

const struct lcd_vmode *get_lcd_vmode(int lcd_id);

