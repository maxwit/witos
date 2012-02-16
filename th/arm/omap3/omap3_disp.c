#include <io.h>
#include <stdio.h>

#define DSS_READL(reg) \
	readl(VA(DSS_BASE + reg))

#define DSS_WRITEL(reg, val) \
	writel(VA(DSS_BASE + reg), val)

static inline void dss_pad_config(void)
{
	void *mux_reg;

	for (mux_reg = VA(0x480020d4); mux_reg <= VA(0x48002108); mux_reg += 2)
		writew(mux_reg, 0x0);
}

#define VIDEO_DMA_BASE  SDRAM_BASE

#define OMAP3_ARGB16  0x5
#define OMAP3_RGB16   0x6
#define OMAP3_ARGB32  0xc
#define OMAP3_RGBA32  0xd

int omap3_set_vmode(void)
{
	__u32 val;
	int i = 0, fmt = OMAP3_RGB16, bpp;
	struct video_mode {
		const char *model;
		int vclk;
		int width, height;
		int hfp, hbp, hpw;
		int vfp, vbp, vpw;
	} svm[] = {
		{
			.model	= "720P",
			.width	= 1280,
			.height = 720,
			.vclk = 1280 * 720 * 60,
			.vbp = 4,
			.vfp = 4,
			.vpw = 2,
			.hbp = 38,
			.hfp = 21,
			.hpw = 6,
		}, {
			.model	= "WXGA",
			.width	= 1280,
			.height = 800,
			.vclk = 1280 * 800 * 60,
			.vbp = 4,
			.vfp = 4,
			.vpw = 2,
			.hbp = 38,
			.hfp = 21,
			.hpw = 6,
		}
	}, *vm = &svm[0];

	i = 0;
	bpp = (fmt + 1) / 3;
	while (i < vm->height * vm->width * bpp / 3) {
		if (OMAP3_RGB16 == fmt)
			writew(VA(VIDEO_DMA_BASE + i), 0x1F << 11);
		else
			writel(VA(VIDEO_DMA_BASE + i), 0xFF << 16);

		i += bpp;
	}

	while (i < vm->height * vm->width * bpp * 2 / 3) {
		if (OMAP3_RGB16 == fmt)
			writew(VA(VIDEO_DMA_BASE + i), 0x1F << 5);
		else
			writel(VA(VIDEO_DMA_BASE + i), 0xFF << 8);

		i += bpp;
	}

	while (i < vm->height * vm->width * bpp) {
		if (OMAP3_RGB16 == fmt)
			writew(VA(VIDEO_DMA_BASE + i), 0xF);
		else
			writel(VA(VIDEO_DMA_BASE + i), 0xFF);

		i += bpp;
	}

	dss_pad_config();

	writel(VA(CM_FCLKEN_DSS), 0x1);
	writel(VA(CM_ICLKEN_DSS), 0x1);

	// __u32 dpll4_clkoutx2;
	// val = readl(VA(CM_CLKSEL2_PLL));
	// dpll4_clkoutx2 = 13 * 2 * 1000 * ((val >> 8) & 0x3FF) / ((val & 0x7F) + 1) * 1000;

	// configure DPLL4 M4
	val = readl(VA(CM_CLKSEL_DSS));
	val &= ~0x1F;
	val |= 5; // dpll4_clkoutx2 / vm->vclk;
	writel(VA(CM_CLKSEL_DSS), val);

	// switch to DSS1_ALWON_FCLK
	val = DSS_READL(VA(DSS_CONTROL));
	val &= ~1;
	DSS_WRITEL(VA(DSS_CONTROL), val);

	// LCD timing
	DSS_WRITEL(DISPC_DIVISOR, 1 << 16 | 3);
	DSS_WRITEL(DISPC_TIMING_H, vm->hbp << 20 | vm->hfp << 8 | vm->hpw);
	DSS_WRITEL(DISPC_TIMING_V, vm->vbp << 20 | vm->vfp << 8 | vm->vpw);
	DSS_WRITEL(DISPC_SIZE_LCD, (vm->height - 1) << 16 | (vm->width - 1));

	// configure GFX
	DSS_WRITEL(DISPC_GFX_BA0, VIDEO_DMA_BASE);
	DSS_WRITEL(DISPC_GFX_POSITION, 0);
	DSS_WRITEL(DISPC_GFX_SIZE, (vm->height - 1) << 16 | (vm->width - 1));
	DSS_WRITEL(DISPC_GFX_ATTRIBUTES, fmt << 1 | 1);

	DSS_WRITEL(DISPC_CONTROL, 3 << 15 | 3 << 8 | 1 << 3 | 1);

	return 0;
}
