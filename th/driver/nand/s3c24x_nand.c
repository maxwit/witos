#include <flash/nand.h>

#define s3c24x_nfc_readl(reg) \
	readl(VA(NAND_CTRL_BASE + reg))

#define s3c24x_nfc_writel(reg, val) \
	writel(VA(NAND_CTRL_BASE + reg), val)

static int s3c24x_nand_ready(struct nand_chip *nand)
{
	return s3c24x_nfc_readl(NF_STAT) & 0x1;
}

int nand_init(struct nand_chip *nand)
{
#ifdef CONFIG_S3C2410
	s3c24x_nfc_writel(NF_CONF, 0x9333);
#elif defined(CONFIG_S3C2440)
	s3c24x_nfc_writel(NF_CONF, 0x2440);
	s3c24x_nfc_writel(NF_CONT, 0x1);
#elif defined(CONFIG_S3C6410)
	s3c24x_nfc_writel(NF_CONF, 0x0214);
	s3c24x_nfc_writel(NF_CONT, 0x1);
#endif

	nand->cmmd_port = VA(NAND_CTRL_BASE + NF_CMMD);
	nand->addr_port = VA(NAND_CTRL_BASE + NF_ADDR);
	nand->data_port = VA(NAND_CTRL_BASE + NF_DATA);

	nand->nand_ready = s3c24x_nand_ready;

	return 0;
}
