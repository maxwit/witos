#include <g-bios.h>
#include <stdio.h>
#include <graphic/gpu.h>

#define BPPMODE   BPP16_TFT
#define FRM565    1

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

#define LCD_BASE  0x77100000
#define lcd_6410_readl(reg) readl(VA(LCD_BASE + reg))
#define lcd_6410_writel(reg, val) writel(VA(LCD_BASE + reg), val)

#define MIFPCON   0x7410800C
#define SPCON     0x7F0081A0
#define GPI_CON   0x7f008100
#define GPJ_CON   0x7f008120

int s3c6410_lcdc_init(void)
{
	u32 dma, val;
	const struct lcd_vmode *vm;
	int clk_val;
	void *va;

	vm = get_lcd_vmode(CONFIG_LCD_ID);
	if (NULL == vm)
	{
		printf("No LCD video mode matched!\n");
		return -ENOENT;
	}
	//set io
	va = video_mem_alloc(vm->width, vm->height, PIX_FMT,&dma);
	if(va == NULL)
	{
		printf("Fail to dma alloc \n");
		goto error;
	}

	writel(VA(GPI_CON), 0xaaaaaaaa);
	writel(VA(GPJ_CON), 0xaaaaaaaa);

	//open clk
	val  = readl(VA(MIFPCON));
	val &= ~(1 << 3);
	writel(VA(MIFPCON), val);

	val  = readl(VA(SPCON));
	val &= ~3;
	val |= 1;
	writel(VA(SPCON), val);
	clk_val = HCLK_RATE / (vm->pixclk * 2) - 1;

	//init register
	lcd_6410_writel(S3C6410_VIDCON0, 0 << 29 | 0 << 26 | 0 << 17 | 0 << 16 | clk_val << 6 | 1 << 4 |0 << 2 | 0x3);
	lcd_6410_writel(S3C6410_VIDCON1, 0 << 5);
	lcd_6410_writel(S3C6410_VIDCON2, 0);

	lcd_6410_writel(S3C6410_VIDTCON0, (vm->vbp - 1)<< 16 | (vm->vfp - 1) << 8 | (vm->vpw - 1));
	lcd_6410_writel(S3C6410_VIDTCON1, (vm->hbp - 1) << 16 | (vm->hfp -1) << 8 | (vm->hpw - 1));
	lcd_6410_writel(S3C6410_VIDTCON2, (vm->height - 1) << 11 | (vm->width - 1));

	lcd_6410_writel(S3C6410_WINCON0, 1 << 16 | 0x05 << 2 | 1);
	lcd_6410_writel(S3C6410_VIDOSD0A, 0);
	lcd_6410_writel(S3C6410_VIDOSD0B, vm->width << 11 | vm->height);
	lcd_6410_writel(S3C6410_VIDOSD0C, vm->width*vm->height);

	//vidosd0a b c0
	lcd_6410_writel(S3C6410_VIDW00ADD0B0, dma);
	lcd_6410_writel(S3C6410_VIDW00ADD1B0, (dma & 0xffffff) + (vm->width + 512)*(vm->height + 1));
	lcd_6410_writel(S3C6410_VIDW00ADD2, vm->width);
	printf("dma= %08x, %p\n", dma, va);

	/*Register fb*/
	return 0;
error:
	return -1;
}

DRIVER_INIT(s3c6410_lcdc_init);
