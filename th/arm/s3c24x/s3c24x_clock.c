/*
 *  comment here
 */

#include <arm/s3c24x0.h>

#define s3c24x0_wdt_writel(reg, val) \
	writel(VA(WATCHDOG_BASE + (reg)), (val))

#define s3c24x0_clock_writel(reg, val) \
	writel(VA(CLOCK_BASE + (reg)), (val))

#define s3c24x0_memcon_writel(reg, val) \
	writel(VA(MEMCON_BASE + (reg)), (val))

int soc_init(void)
{
	asm volatile ("mov sp, %0\n"::"i"(S3C24XX_SRAM_SIZE));

	// WDT
	s3c24x0_wdt_writel(WTCON, 0x0);

	// Clock
	s3c24x0_clock_writel(LOCKTIME, 0xffffff);
	s3c24x0_clock_writel(MPLLCON, MDIV << 12 | PDIV << 4 | SDIV);
	s3c24x0_clock_writel(CLKDIVN, HDIVN << 1 | PDIVN);

	return 0;
}

static const __u32 g_mem_conf[] = {
#ifdef CONFIG_S3C2410
	0x2200d000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00001f4c,
	0x00000000,
	0x00000000,
	0x00018001,
	0x00018001,
	0x008404e8,
	0x00000011,
	0x00000020,
	0x00000020,
#elif defined(CONFIG_S3C2440)
	0x220dd000,
	0x00000700,
	0x00000700,
	0x00000700,
	0x00000700,
	0x00000700,
	0x00000700,
	0x00018009,
	0x00018009,
	0x00ac03f4,
	0x000000b2,
	0x00000030,
	0x00000030
#endif
};

// to be improved
int mem_init(void)
{
	int i;

	for (i = 0; i < ARRAY_ELEM_NUM(g_mem_conf); i++)
		s3c24x0_memcon_writel(4 * i, g_mem_conf[i]);

	// for (i = 0; i < 256; i++);

	return SDRAM_BASE + SDRAM_SIZE;
}

