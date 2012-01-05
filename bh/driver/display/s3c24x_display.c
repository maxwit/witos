#include <graphic/display.h>

//
#define PNRMODE   3
#define BPPMODE   BPP16_TFT
#define FRM565    1

#define s3c24x0_lcdc_writel(reg, val) \
	writel(VA(S3C24X0_LCD_BASE + reg), val)

#define s3c24x0_lcdc_readl(reg) \
	readl(VA(S3C24X0_LCD_BASE + reg))

static int __INIT__ s3c24x0_lcdc_init(void)
{
	__u32 video_buff_dma;
	__u16 *video_buff_cpu;
	const struct lcd_vmode *vm;
	int clk_val;

	writel(VA(S3C24X0_GPCCON), 0xaaaaaaaa);
	writel(VA(S3C24X0_GPDCON), 0xaaaaaaaa);

#if BPPMODE == BPP16_TFT
	#if FRM565 == 1
		#define PIX_FMT  PIX_RGB16
	#else
		#define PIX_FMT  PIX_RGB15
	#endif
#elif BPPMODE == BPP24_TFT
	#define PIX_FMT PIX_RGB24
#else
	#error
#endif

	vm = lcd_get_vmode_by_name(CONFIG_LCD_MODEL);
	if (NULL == vm) {
		printf("No LCD video mode matched!\n");
		return -ENOENT;
	}

	// alloc and init the video buffer:
	video_buff_cpu = video_mem_alloc(&video_buff_dma, vm, PIX_FMT);
	if (NULL == video_buff_cpu)
		return -ENOMEM;

#if 0
	val = readl(VA(0x4c00000c));
	val |= 0x20;
	writel(VA(0x4c00000c), val);
#endif

	s3c24x0_lcdc_writel(LCDSADDR1, video_buff_dma >> 1);
	s3c24x0_lcdc_writel(LCDSADDR2, ((video_buff_dma >> 1) & 0x1fffff) + vm->width * vm->height);
	s3c24x0_lcdc_writel(LCDSADDR3, vm->width);

	clk_val = HCLK_RATE / (vm->pix_clk * 2) - 1;

	s3c24x0_lcdc_writel(LCDCON1, clk_val << 8 | PNRMODE << 5 | BPPMODE << 1 | 1);
	s3c24x0_lcdc_writel(LCDCON2, (vm->vbp - 1) << 24 | (vm->height - 1) << 14 | (vm->vfp - 1) << 6 | (vm->vpw - 1));
	s3c24x0_lcdc_writel(LCDCON3, (vm->hbp - 1) << 19 | (vm->width - 1) << 8 | (vm->hfp - 1));
	s3c24x0_lcdc_writel(LCDCON4, vm->hpw - 1);
	s3c24x0_lcdc_writel(LCDCON5, FRM565 << 11 | 0 << 10 | 1 << 9 | 1 << 8 | 1 << 3 | 1);

	// s3c24x0_lcdc_writel(TCONSEL, 0);

	return 0;
}

module_init(s3c24x0_lcdc_init);

