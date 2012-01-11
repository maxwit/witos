/*
 *  comment here
 */

#include <flash/nand.h>

int at91_nand_ready(struct nand_chip *nand)
{
	return readl(VA(PIO_NAND + PIO_PDSR)) & PIO_NAND_RDY;
}

int nand_init(struct nand_chip *nand)
{
	__u32 val;

#ifdef CONFIG_AT91SAM9261
	val = readl(VA(AT91SAM926X_PA_MATRIX + MATRIX_EBICSA));
	val |= 1 << 3;
	writel(VA(AT91SAM926X_PA_MATRIX + MATRIX_EBICSA), val);

	at91_smc_writel(SMC_SETUP(3), 0);
	at91_smc_writel(SMC_PULSE(3), 4 | 6 << 8 | 3 << 16 | 5 << 24);
	at91_smc_writel(SMC_CYCLE(3), 6 | 5 << 16);
	at91_smc_writel(SMC_MODE(3), 1 | 1 << 1 | 1 << 16);

	writel(VA(AT91SAM926X_PA_PMC + PMC_PCER), 1 << PID_PIOC);

	// GPIO
	writel(VA(AT91SAM926X_PA_PIOC + PIO_PDR), 0x3);
	writel(VA(AT91SAM926X_PA_PIOC + PIO_ASR), 0x3);

	writel(VA(AT91SAM926X_PA_PIOC + PIO_PER), PIO_NAND_CE);
	writel(VA(AT91SAM926X_PA_PIOC + PIO_OER), PIO_NAND_CE);
	writel(VA(AT91SAM926X_PA_PIOC + PIO_CODR), PIO_NAND_CE);

	writel(VA(AT91SAM926X_PA_PIOC + PIO_PER), PIO_NAND_RDY);
	writel(VA(AT91SAM926X_PA_PIOC + PIO_ODR), PIO_NAND_RDY);
	writel(VA(AT91SAM926X_PA_PIOC + PIO_PUER), PIO_NAND_RDY);
#elif defined(CONFIG_AT91SAM9263)
	val = readl(VA(AT91SAM926X_PA_MATRIX + MATRIX_EBI0CSA));
	val |= 1 << 3;
	writel(VA(AT91SAM926X_PA_MATRIX + MATRIX_EBI0CSA), val);

	at91_smc_writel(SMC_SETUP(3), 0);
	at91_smc_writel(SMC_PULSE(3), 4 | 6 << 8 | 3 << 16 | 5 << 24);
	at91_smc_writel(SMC_CYCLE(3), 6 | 5 << 16);
	at91_smc_writel(SMC_MODE(3), 1 | 1 << 1 | 1 << 16);

	at91_pmc_writel(PMC_PCER, 1 << PID_PIOA | 1 << PID_PIOD);

	at91_gpio_conf_output(PIOD, PIO_NAND_CE, 0);
	at91_gpio_conf_input(PIOA, PIO_NAND_RDY, 1);

	writel(VA(AT91SAM926X_PA_PIOD + PIO_CODR), PIO_NAND_CE);

	writel(VA(AT91SAM926X_HECC0_MR), 0x2);
#endif

	nand->cmmd_port = VA(AT91SAM926X_PA_NAND + NAND_CMMD);
	nand->addr_port = VA(AT91SAM926X_PA_NAND + NAND_ADDR);
	nand->data_port = VA(AT91SAM926X_PA_NAND + NAND_DATA);

	nand->nand_ready = at91_nand_ready;

	return 0;
}

