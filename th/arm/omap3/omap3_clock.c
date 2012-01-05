#include <io.h>
#include <arm/omap3.h>

int soc_init(void)
{
	__u32 word;

	// disable WDT
	writel(VA(CM_FCLKEN_WKUP), 1 << 5);
	writel(VA(CM_ICLKEN_WKUP), 1 << 5);
	while (readl(VA(CM_IDLEST_WKUP)) & (1 << 5));

	writel(VA(WDTTIMER2 + WSPR), 0xaaaa);
	while (readl(VA(WDTTIMER2 + WWPS)));
	writel(VA(WDTTIMER2 + WSPR), 0x5555);

	// init SP
	word = OMAP3_SRAM_BASE + OMAP3_SRAM_SIZE;

	asm volatile ("mov sp, %0\n\t"::"r"(word));

	// System clock input selection: 26MHz
	word = 0x3;
	writel(VA(PRM_CLKSEL), word);

	// Input clock divider 2.
	word = readl(VA(PRM_CLKSRC_CTRL));
	word &= ~(0x3 << 6);
	word |= 0x2 << 6;
	writel(VA(PRM_CLKSRC_CTRL), word);

	//	CLKOUTX2 = (SYS_CLK * M ) / ([N+1])
	//			    13	      250	     12
	//	MPU_CLK	=  CLKOUTX2  / M2
	//						    1
	word = readl(VA(CM_CLKSEL1_PLL_MPU));
	word &= ~(0x7FF << 8 | 0x7F);
	word |= 0xFA << 8 | 0xC;
	writel(VA(CM_CLKSEL1_PLL_MPU), word);

	word = readl(VA(CM_CLKSEL2_PLL_MPU));
	word &= ~(0x1F);
	word |= 0x1;
	writel(VA(CM_CLKSEL2_PLL_MPU), word);

	// lock mode.
	word = readl(VA(CM_CLKEN_PLL_MPU));
	word |= 0x7;
	writel(VA(CM_CLKEN_PLL_MPU), word);

	// DPLL3.
	// fCORE_CLK  = (fSYS_CLK x M) / ((N+1) * M2)
	// 332	                  13	 332     12      1
	word = readl(VA(CM_CLKSEL1_PLL));
	word &= ~(0x7FF << 16);
	word |= 0x14C << 16;
	writel(VA(CM_CLKSEL1_PLL), word);

	word = readl(VA(CM_CLKSEL1_PLL));
	word &= ~(0x7F << 8);
	word |= 0xC << 8;
	writel(VA(CM_CLKSEL1_PLL), word);

	word = readl(VA(CM_CLKSEL1_PLL));
	word &= ~(0xF << 27);
	word |= 0x1 << 27;
	writel(VA(CM_CLKSEL1_PLL), word);

	// fL4_ICLK = fCORE_CLK / DIV_L4
	// 166M            332M       2
	word = readl(VA(CM_CLKSEL_CORE));
	word &= ~(0x3 <<2);
	word |= 0x2 << 2;
	writel(VA(CM_CLKSEL_CORE), word);

	// fL3_ICLK = fCORE_CLK / DIV_L3
	// 166M            332M       2
	word = readl(VA(CM_CLKSEL_CORE));
	word &= ~(0x3);
	word |= 0x2;
	writel(VA(CM_CLKSEL_CORE), word);

	// lock mode.
	word = readl(VA(CM_CLKEN_PLL));
	word |= 0x7;
	writel(VA(CM_CLKEN_PLL), word);

	//
#if 1
	word = readl(VA(CM_ICLKEN_WKUP));
	word |= 3;
	writel(VA(CM_ICLKEN_WKUP), word);

	word = readl(VA(CM_FCLKEN_WKUP));
	word |= 1;
	writel(VA(CM_FCLKEN_WKUP), word);
#endif

	word = readl(VA(CM_CLKSEL_PER));
	word |= 1 << 0;
	writel(VA(CM_CLKSEL_PER), word);

	word = readl(VA(CM_ICLKEN_PER));
	word |= 1 << 3;
	writel(VA(CM_ICLKEN_PER), word);

	readl(VA(CM_FCLKEN_PER));
	word |= 1 << 3;
	writel(VA(CM_FCLKEN_PER), word);

	/* UART 3 Clocks */
	readl(VA(CM_FCLKEN_PER));
	word |= 1 << 11;
	writel(VA(CM_FCLKEN_PER), word);

	readl(VA(CM_ICLKEN_PER));
	word |= 1 << 11;
	writel(VA(CM_ICLKEN_PER), word);

	return 0;
}

#define SDRC_READL(reg) readl(VA(SDRC_BASE + (reg)))
#define SDRC_WRITEL(reg, word) writel(VA(SDRC_BASE + (reg)), (word))

int mem_init(void)
{
	int i;

	SDRC_WRITEL(SDRC_SYSCONFIG, 0x1 << 1);
	while (!(SDRC_READL(SDRC_SYSSTATUS) & 0x1));
	SDRC_WRITEL(SDRC_SYSCONFIG, 0x0);

	SDRC_WRITEL(SDRC_SHARING, 0x100);

	SDRC_WRITEL(SDRC_MCFG_0, 0x02584019);

	SDRC_WRITEL(SDRC_ACTIM_CTRLA_0, 21 << 27 | 10 << 22 | 7 << 18 | 3 << 15 \
								   | 3 << 12 | 2 << 9 | 3 << 6 | 6);

	SDRC_WRITEL(SDRC_ACTIM_CTRLB_0, 1 << 12 | 23 << 0 | 5 << 8 | 1 << 16);

	SDRC_WRITEL(SDRC_RFR_CTRL, 0x0004e201);

	SDRC_WRITEL(SDRC_POWER, 0x81);

	SDRC_WRITEL(SDRC_MANUAL_0, 0x0);

	i = 0;
	while (i++ < 0x500000);

	SDRC_WRITEL(SDRC_MANUAL_0, 0x1);
	SDRC_WRITEL(SDRC_MANUAL_0, 0x2);
	SDRC_WRITEL(SDRC_MANUAL_0, 0x2);

	SDRC_WRITEL(SDRC_MR_0, 0x32);

	SDRC_WRITEL(SDRC_DLLA_CTRL, 1 << 3 | 0 << 2 | 1 << 1);

	i = 0;
	while (i++ < 0x50000);

	return SDRAM_BASE + SDRAM_SIZE;
}
