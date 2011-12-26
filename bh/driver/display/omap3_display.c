#include <stdio.h>
#include <graphic/display.h>

#define lcd_omap3_readl(reg) \
	readl(VA(DISPC_BASE + reg))  // fix

#define lcd_omap3_writel(reg, val) \
	writel(VA(DISPC_BASE + reg), val) //fix

static int omap3_set_vmode(struct display *disp, const struct lcd_vmode *vm)
{
	__u32 fmt;
	// __u32 bpp;
	__u32 dma = disp->video_mem_pa;

	// fixme
	switch (disp->pix_fmt) {
	case PIX_RGB16:
		fmt = 0x6;
		// bpp = 2;
		break;

	default:
		BUG();
	}

	lcd_omap3_writel(DISPC_SYSCONFIG, 1);
	lcd_omap3_writel(DISPC_CONFIG, 1 << 9 | 1 << 1);

	lcd_omap3_writel(DISPC_TIMING_H, vm->hbp << 20 | vm->hfp << 8 | vm->hpw);
	lcd_omap3_writel(DISPC_TIMING_V, vm->vbp << 20 | vm->vfp << 8 | vm->vpw);

	lcd_omap3_writel(DISPC_DIVISOR, 1 << 16 | 2); // fix me
	lcd_omap3_writel(DISPC_SIZE_LCD, (vm->height - 1) << 16 | (vm->width - 1));

	lcd_omap3_writel(DISPC_GFX_BA0, dma);
	lcd_omap3_writel(DISPC_GFX_POSITION, 0);
	lcd_omap3_writel(DISPC_GFX_SIZE, (vm->height - 1) << 16 | (vm->width - 1));
	lcd_omap3_writel(DISPC_GFX_ATTRIBUTES, fmt << 1 | 1);

	lcd_omap3_writel(DISPC_VID0_BA0, dma);
	lcd_omap3_writel(DISPC_VID0_POSITION, 0);
	lcd_omap3_writel(DISPC_VID0_SIZE, vm->height << 16 | vm->width);
	lcd_omap3_writel(DISPC_VID0_ATTRIBUTES, 3 << 5 | 6 << 1 | 1);

	lcd_omap3_writel(DISPC_CONTROL,  1 << 16 | 1 << 15 | 1 << 8 | 1 << 3 | 1);

	disp->video_mode = (struct lcd_vmode *)vm;

	return 0;
}

static int __INIT__ omap3_display_init(void)
{
	int ret;
	struct display *disp;

	// writel(VA(0x48004e00), 7);
	// writel(VA(0x48004e10), 1);

	disp = display_create();
	// if NULL

	ret = display_config(disp, omap3_set_vmode);
	if (ret < 0)
		return ret;

	display_register(disp);

	return 0;
}

DRIVER_INIT(omap3_display_init);
