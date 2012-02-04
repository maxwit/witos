#include <io.h>
#include <init.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <graphic/display.h>

#define lcd_omap3_readl(reg) \
	readl(VA(DISPC_BASE + reg))  // fix

#define lcd_omap3_writel(reg, val) \
	writel(VA(DISPC_BASE + reg), val) //fix

static int omap3_set_vmode(struct display *disp, const struct lcd_vmode *vm)
{
	__u32 fmt;
	__u32 dma = disp->video_mem_pa;

	// fixme
	switch (disp->pix_fmt) {
	case PIX_RGB16:
		fmt = 0x6;
		break;

	case PIX_RGB24:
		fmt = 0x9;
		break;

	default:
		printf("%s(): invalid pixel format (%d)!\n", __func__, disp->pix_fmt);
		return -EINVAL;
	}

	lcd_omap3_writel(DISPC_SYSCONFIG, 1);
	lcd_omap3_writel(DISPC_CONFIG, 1 << 9 | 1 << 1);

	lcd_omap3_writel(DISPC_TIMING_H, vm->hbp << 20 | vm->hfp << 8 | vm->hpw);
	lcd_omap3_writel(DISPC_TIMING_V, vm->vbp << 20 | vm->vfp << 8 | vm->vpw);

	lcd_omap3_writel(DISPC_DIVISOR, 1 << 16 | 2); // fixme
	lcd_omap3_writel(DISPC_SIZE_LCD, (vm->height - 1) << 16 | (vm->width - 1));

	lcd_omap3_writel(DISPC_GFX_BA0, dma);
	lcd_omap3_writel(DISPC_GFX_POSITION, 0);
	lcd_omap3_writel(DISPC_GFX_SIZE, (vm->height - 1) << 16 | (vm->width - 1));
	lcd_omap3_writel(DISPC_GFX_ATTRIBUTES, fmt << 1 | 1);

#if 0
	lcd_omap3_writel(DISPC_VID0_BA0, dma);
	lcd_omap3_writel(DISPC_VID0_POSITION, 0);
	lcd_omap3_writel(DISPC_VID0_SIZE, vm->height << 16 | vm->width);
	lcd_omap3_writel(DISPC_VID0_ATTRIBUTES, 3 << 5 | fmt << 1 | 1);
#endif

	lcd_omap3_writel(DISPC_CONTROL,  1 << 16 | 1 << 15 | 1 << 8 | 1 << 3 | 1);

	disp->video_mode = (struct lcd_vmode *)vm;

	return 0;
}

static int __INIT__ omap3_display_init(void)
{
	int ret;
	// void *va;
	// unsigned long dma;
	// const struct lcd_vmode *vm;
	struct display *disp;
	// char model[CONF_VAL_LEN];

	// writel(VA(0x48004e00), 7);
	// writel(VA(0x48004e10), 1);

	disp = display_create();
	// if NULL

#if 0
	ret = conf_get_attr("dispplay.lcd.model", model);
	if (ret < 0)
		goto error;

	vm = lcd_get_vmode_by_name(model);
	if (NULL == vm) {
		printf("No LCD video mode found!\n");
		return -ENOENT;
	}

	va = video_mem_alloc(&dma, vm, disp->pix_fmt);
	if(va == NULL) {
		GEN_DBG("video_mem_alloc() failed\n");
		goto error;
	}

	disp->video_mem_va = va;
	disp->video_mem_pa = dma;
	disp->set_vmode    = omap3_set_vmode;

	ret = omap3_set_vmode(disp, vm);
	if (ret < 0)
		goto error;
#else
	disp->set_vmode = omap3_set_vmode;
#endif

	ret = display_register(disp);
	if (ret < 0)
		goto error;

	return 0;

error:
	// TODO:
	return ret;
}

module_init(omap3_display_init);
