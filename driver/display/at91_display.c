#include <io.h>
#include <init.h>
#include <stdio.h>
#include <errno.h>
#include <graphic/display.h>

#define INVVD         8
#define INVFRAME      9
#define INVLINE       10
#define INVCLK        11
#define INVDVAL       12
#define CLKMOD        15
#define MEMOR         30
#define VHDLY         1
#define PIX_BPP       15
#define PIX_FMT       PIX_RGB15

static inline __u32 at91_lcdc_readl(int reg)
{
	return readl(VA(AT91SAM926X_PA_LCDC + reg));
}

static inline void at91_lcdc_writel(int reg, int val)
{
	writel(VA(AT91SAM926X_PA_LCDC + reg), val);
}

static int at91_set_vmode(struct display *disp, const struct lcd_vmode *vm)
{
	writel(VA(AT91SAM926X_PA_PIOB + PIO_PDR), 0xffffffff);
	writel(VA(AT91SAM926X_PA_PIOB + PIO_ASR), 0xffff);
	writel(VA(AT91SAM926X_PA_PIOB + PIO_BSR), 0x3f << 23);

	writel(VA(AT91SAM926X_PA_PMC + PMC_SCER), 0x3 << 16);
	writel(VA(AT91SAM926X_PA_PMC + PMC_PCER), 1 << 21);

	at91_lcdc_writel(LCDCON1, 7 << 12);
	at91_lcdc_writel(LCDCON2, 2 | 0 << 2 | 4 << 5
			| 0 << INVVD
			| 1 << INVFRAME
			| 1 << INVLINE
			| 1 << INVCLK
			| 0 << INVDVAL
			| 1 << CLKMOD
			| 2 << MEMOR);
	at91_lcdc_writel(LCDTIM1, (VHDLY - 1) << 24
			| (vm->vpw - 1) << 16
			| (vm->vbp - 1) << 8
			| (vm->vfp - 1));
	at91_lcdc_writel(LCDTIM2, (vm->hfp - 1) << 21
			| (vm->hpw - 1) << 8
			| (vm->hbp - 1));
	at91_lcdc_writel(LCDFRMCFG, (vm->width - 1) << 21 | (vm->height - 1));
	at91_lcdc_writel(LCDFIFO, 501);

	at91_lcdc_writel(DMABADDR1, disp->video_mem_pa);
	at91_lcdc_writel(DMAFRMCFG, (4 -1) << 24
			| (vm->width * vm->height) * ((PIX_BPP +7)/8)/4);
	at91_lcdc_writel(DMACON, 1);

	at91_lcdc_writel(PWRCON, 30 << 1 | 1);

	return 0;
}

static int __init at91_lcdc_init(void)
{
	int ret;
	struct display *disp;

	disp = display_create();
	// if null

	disp->set_vmode = at91_set_vmode;

	ret = display_register(disp);
	if (ret < 0)
		goto error;

	return 0;

error:
	display_destroy(disp);
	return ret;
}

module_init(at91_lcdc_init);
