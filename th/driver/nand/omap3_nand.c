#include <io.h>
#include <delay.h>
#include <flash/nand.h>

static int omap3_nand_ready(struct nand_chip *nand)
{
	return readl(VA(GPMC_BASE + GPMC_STATUS)) & (1 << 8);
}

int nand_init(struct nand_chip *nand)
{
	writel(VA(GPMC_BASE + SYSCONFIG), 0x10);
	writel(VA(GPMC_BASE + IRQENABLE), 0x0);
	writel(VA(GPMC_BASE + TIMEOUT), 0x0);
	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_7), 0x0);

	udelay(0x100);

	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_1), 0x1800);
	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_2), 0x00141400);
	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_3), 0x00141400);
	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_4), 0x0F010F01);
	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_5), 0x010C1414);
	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_6), 0x1F0F0A80);

	writel(VA(GPMC_BASE + GPMC_CONFIG_CS0 + GPMC_CONFIG_7), (0x8 & 0xf) << 8 | ((0x0c000000 >> 24) & 0x3f) | 1 << 6);

	udelay(0x100);

	nand->cmmd_port = VA(GPMC_BASE + GPMC_NAND_COMMAND_0);
	nand->addr_port = VA(GPMC_BASE + GPMC_NAND_ADDRESS_0);
	nand->data_port = VA(GPMC_BASE + GPMC_NAND_DATA_0);
	nand->nand_ready = omap3_nand_ready;

	return 0;
}
