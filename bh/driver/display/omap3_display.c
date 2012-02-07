#include <io.h>
#include <init.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <graphic/display.h>

#define omap3_disp_readl(reg) \
	readl(VA(DISPC_BASE + reg))

#define omap3_disp_writel(reg, val) \
	writel(VA(DISPC_BASE + reg), val)

static int omap3_set_vmode(struct display *disp, const struct lcd_vmode *vm)
{
	__u32 fmt;

	omap3_disp_writel(DISPC_SYSCONFIG, 1);
	omap3_disp_writel(DISPC_CONFIG, 1 << 9 | 1 << 1);

	omap3_disp_writel(DISPC_TIMING_H, vm->hbp << 20 | vm->hfp << 8 | vm->hpw);
	omap3_disp_writel(DISPC_TIMING_V, vm->vbp << 20 | vm->vfp << 8 | vm->vpw);

	omap3_disp_writel(DISPC_DIVISOR, 1 << 16 | 2); // fixme

	omap3_disp_writel(DISPC_SIZE_LCD, (vm->height - 1) << 16 | (vm->width - 1));

	omap3_disp_writel(DISPC_GFX_BA0, disp->video_mem_pa);
	omap3_disp_writel(DISPC_GFX_POSITION, 0);
	omap3_disp_writel(DISPC_GFX_SIZE, (vm->height - 1) << 16 | (vm->width - 1));

	// fixme
	switch (disp->pix_fmt) {
	case PIX_RGB16:
		fmt = 0x6;
		break;

	case PIX_RGB24:
		fmt = 0x9;
		break;

	case PIX_RGB32:
		fmt = 0xd;
		break;

	default:
		printf("%s(): invalid pixel format (%d)!\n", __func__, disp->pix_fmt);
		return -EINVAL;
	}

	omap3_disp_writel(DISPC_GFX_ATTRIBUTES, fmt << 1 | 1);

#if 0
	omap3_disp_writel(DISPC_VID0_BA0, disp->video_mem_pa);
	omap3_disp_writel(DISPC_VID0_POSITION, 0);
	omap3_disp_writel(DISPC_VID0_SIZE, vm->height << 16 | vm->width);
	omap3_disp_writel(DISPC_VID0_ATTRIBUTES, 3 << 5 | fmt << 1 | 1);
#endif

	omap3_disp_writel(DISPC_CONTROL, 1 << 16 | 1 << 15 | 1 << 8 | 1 << 3 | 1);

	return 0;
}

static int __INIT__ omap3_display_init(void)
{
	int ret;
	struct display *disp;

	disp = display_create();
	// if NULL

	// writel(VA(0x48004e00), 7);
	// writel(VA(0x48004e10), 1);

	disp->set_vmode = omap3_set_vmode;

	ret = display_register(disp);
	if (ret < 0)
		goto error;

	return 0;

error:
	display_destroy(disp);
	return ret;
}

module_init(omap3_display_init);
