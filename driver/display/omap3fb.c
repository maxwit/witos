#include <fb.h>
#include <platform.h>

#define dss_readl(reg) \
	readl(omapfb->mmio + reg)

#define dss_writel(reg, val) \
	writel(omapfb->mmio + reg, val)

struct omapfb_info {
	struct omapfb_panel *panel;
	void *mmio;
};

static int omapfb_check_var(struct fb_info *fb,
	struct fb_var_screeninfo *var)
{
	struct omapfb_info *omapfb = fb->par;

	if (var->xres != omapfb->panel->width || var->yres != omapfb->panel->height)
		return -ENOTSUPP;

	fb->var = *var;

	return 0;
}

static int omapfb_set_par(struct fb_info *fb)
{
	struct fb_var_screeninfo *var = &fb->var;
	struct omapfb_info *omapfb = fb->par;

	dss_writel(DISPC_SIZE_LCD, (var->xres - 1) << 16 | (var->yres - 1));
}

static const struct fb_ops omapfb_ops = {
	.fb_check_var = omapfb_check_var,
	.fb_set_par = omapfb_set_par,
};

static int __init omapfb_probe(struct platform_device *pdev)
{
	int ret;
	struct fb_info *fb;
	struct omapfb_info *omapfb;
	struct resource *mres;

	mres = platform_get_mem(pdev, 0);
	if (!mres)
		return -ENODEV;

	fb = framebuffer_alloc(sizeof(*omapfb));
	// ...

	omapfb = fb->par;

	omapfb->mmio = VA(mres->start);
	omapfb->panel = pdev->platform_data;
	if (!omapfb->panel)
		return -ENODEV;

	fb->ops = &omapfb_ops;

	ret = framebuffer_register(fb);
	// if ret < 0 ...

	return 0;
}

static struct platform_driver omapfb_driver = {
	.drv = {
		.name = "omap disp",
	},
	.probe = omapfb_probe,
};

static int __init omapfb_init(void)
{
	return platform_driver_register(&omapfb_driver);
}

module_init(omapfb_init);
