#include <flash/flash.h>
#include <flash/nand.h>

static int __INIT__ at91_nand_init(void)
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

	return 0;
}

static void at91_nand_cmd(struct nand_chip *nand, int cmd, unsigned int ctrl)
{
	struct nand_ctrl *nfc;

	if (cmd == NAND_CMMD_NONE)
		return;

	nfc = nand->master;

	if (ctrl & NAND_CLE)
		writeb((char *)nfc->data_reg + NAND_CMMD, cmd);
	else
		writeb((char *)nfc->data_reg + NAND_ADDR, cmd);
}

static int at91_nand_ready(struct nand_chip *nand)
{
#if defined(CONFIG_AT91SAM9261)
	return readl(VA(AT91SAM926X_PA_PIOC + PIO_PDSR)) & PIO_NAND_RDY;
#elif defined(CONFIG_AT91SAM9263)
	return readl(VA(AT91SAM926X_PA_PIOA + PIO_PDSR)) & PIO_NAND_RDY;
#endif
}

static int __INIT__ at91_nand_probe(void)
{
	int ret;
	struct nand_ctrl *nfc;

	nfc = nand_ctrl_new();

	if (NULL == nfc)
		return -ENOMEM;

	nfc->data_reg = VA(AT91SAM926X_PA_NAND);
	nfc->cmmd_reg = VA(AT91SAM926X_PA_NAND + NAND_CMMD);
	nfc->addr_reg = VA(AT91SAM926X_PA_NAND + NAND_ADDR);

	nfc->name = "atmel_nand";

	nfc->flash_ready = at91_nand_ready;
	nfc->cmd_ctrl = at91_nand_cmd;

	at91_nand_init();

	ret = nand_ctrl_register(nfc);
	if (ret < 0) {
		ret = -ENODEV;
		goto L1;
	}

	return 0;

L1:
	free(nfc);

	return ret;
}

module_init(at91_nand_probe);
