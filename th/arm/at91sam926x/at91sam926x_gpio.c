#include <g-bios.h>


int at91_clock_enable(int nClockID)
{
	writel(VA(AT91SAM926X_PA_PMC + PMC_PCER), 1 << nClockID);
	return 0;
}


int at91_clock_disable(int nClockID)
{
	writel(VA(AT91SAM926X_PA_PMC + PMC_PCDR), 1 << nClockID);
	return 0;
}


void at91_gpio_conf_periA(u32 nPioIdx, u32 mask, int isPullUp)
{
	writel(VA(PIO_BASE(nPioIdx)) + PIO_PDR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + PIO_ASR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + PIO_IDR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + (isPullUp ? PIO_PUER : PIO_PUDR), mask);
}


void at91_gpio_conf_periB(u32 nPioIdx, u32 mask, int isPullUp)
{
	writel(VA(PIO_BASE(nPioIdx)) + PIO_PDR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + PIO_BSR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + PIO_IDR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + (isPullUp ? PIO_PUER : PIO_PUDR), mask);
}


void  at91_gpio_conf_input(u32 nPioIdx, u32 mask, int isPullUp)
{
	writel(VA(PIO_BASE(nPioIdx)) + PIO_PER, mask);
	writel(VA(PIO_BASE(nPioIdx)) + PIO_ODR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + PIO_IDR, mask);
	writel(VA(PIO_BASE(nPioIdx)) + (isPullUp ? PIO_PUER : PIO_PUDR), mask);
}


void  at91_gpio_conf_output(u32 nPioIdx, u32 mask, int isPullUp)
{
	writel(VA(PIO_BASE(nPioIdx) + PIO_PER), mask);
	writel(VA(PIO_BASE(nPioIdx) + PIO_OER), mask);
	writel(VA(PIO_BASE(nPioIdx) + PIO_IDR), mask);
	writel(VA(PIO_BASE(nPioIdx) + (isPullUp ? PIO_PUER : PIO_PUDR)), mask);
}

