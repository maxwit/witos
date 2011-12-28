#include <arm/s3c6410.h>

// init clock, gpio, wdt, etc.
int soc_init(void)
{
	__u32 val = S3C6410_SRAM_BASE + S3C6410_SRAM_SIZE;

	asm volatile ("mov sp, %0\n\t"::"r"(val));

	// configure GPIO
#ifdef CONFIG_DEBUG
	val = readl(VA(0x7F0080A0));
	val &= ~(3 << 30);
	val |= 1 << 30;
	writel(VA(0x7F0080A0), val);

	val = readl(VA(0x7F0080A4));
	val |= 1 << 15;
	writel(VA(0x7F0080A4), val);
#endif

	val = readl(VA(0x7F008820));
	val &= ~0xFFFF;
	val |= 0x1111;
	writel(VA(0x7F008820), val);

	val = readl(VA(0x7F008824));
	val &= ~0xF;
	val |= 0x6;
	writel(VA(0x7F008824), val);

	// init Clock
	syscon_write(APLL_LOCK, 0xE11);
	syscon_write(MPLL_LOCK, 0xE11);

	syscon_write(APLL_CON, 1 << 31 | A_MDIV << 16 | A_PDIV << 8 | A_SDIV);
	syscon_write(MPLL_CON, 1 << 31 | M_MDIV << 16 | M_PDIV << 8 | M_SDIV);

	syscon_write(CLK_SRC, 0x2007);
	syscon_write(OTHERS, 0x0);
	syscon_write(MISC_CON, 0x0);
	syscon_write(CLK_DIV0, 0x3100);

	return 0;
}

#define ddr_write(reg, val) writel(VA(DRAMC_BASE + reg), val)
#define ddr_read(reg)       readl(VA(DRAMC_BASE + reg))

#define CASL      3      // DDR266
#define tRC       68
#define tRAS      45     // max 70,000
#define tRCD      23
#define tRP       23
#define tRRD      15
#define tWR       15
#define tREF      6400   // max
#define tRFC      80
#define tXSR      120
#define tDQSS     1
#define tMRD      2

#define MCMD_PRE    0
#define MCMD_REF    1
#define MCMD_MRS    2
#define MCMD_NOP    3

// fixme
#define TIME2CYCLE(t) (HCLK_RATE / 1000 * (t) / 1000000)
#define TIME_SUB3(t)  ((t) <= 3 ? 0 : (t) - 3)

int mem_init(void)
{
	__u32 val;
	__u32 rcd, rfc, rp;

	ddr_write(P1MEMCCMD, 0x4);

	ddr_write(P1REFRESH, TIME2CYCLE(tREF));
	ddr_write(P1CASLAT, CASL << 1);
	ddr_write(P1T_DQSS, tDQSS);
	ddr_write(P1T_MRD, tMRD);
	ddr_write(P1T_RAS, TIME2CYCLE(tRAS));
	ddr_write(P1T_RC, TIME2CYCLE(tRC));

	rcd = TIME2CYCLE(tRCD);
	ddr_write(P1T_RCD, TIME_SUB3(rcd) << 3 | rcd);

	rfc = TIME2CYCLE(tRFC);
	ddr_write(P1T_RFC, TIME_SUB3(rfc) << 3 | rfc);

	rp = TIME2CYCLE(tRP);
	ddr_write(P1T_RP, TIME_SUB3(rp) << 3 | rp);

	ddr_write(P1T_RRD, TIME2CYCLE(tRRD));
	ddr_write(P1T_WR, TIME2CYCLE(tWR));
	ddr_write(P1T_WTR, 2);
	ddr_write(P1T_XP, 2);

	ddr_write(P1T_XSR, TIME2CYCLE(tXSR));
	ddr_write(P1T_ESR, TIME2CYCLE(tXSR));

	ddr_write(P1MEMCFG, 2 << 15 | 2 << 3 | 2);
	ddr_write(P1MEMCFG2, 1 << 11 | 3 << 8 | 1 << 6 | 1);

	ddr_write(P1CHIP_0_CFG, 1 << 16 | 0x50 << 8 | 0xf8);

	//
	ddr_write(P1DIRECTCMD, MCMD_NOP << 18);
	ddr_write(P1DIRECTCMD, MCMD_PRE << 18);
	ddr_write(P1DIRECTCMD, MCMD_REF << 18);
	ddr_write(P1DIRECTCMD, MCMD_REF << 18);
	ddr_write(P1DIRECTCMD, MCMD_MRS << 18 | 0x32);
	ddr_write(P1DIRECTCMD, MCMD_MRS << 18 | 2 << 16);

	ddr_write(P1MEMCCMD, 0);

	while((ddr_read(P1MEMSTAT) & 0x3) != 1);

	val = readl(VA(0x7E00F120));
	val |= 0x1000;
	val &= ~0xbf;
	writel(VA(0x7E00F120), val);

	return SDRAM_BASE + SDRAM_SIZE;
}
