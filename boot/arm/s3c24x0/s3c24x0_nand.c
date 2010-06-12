#include <g-bios.h>
#include <flash/nand.h>


static int s3c24x0_nand_ready(struct nand_chip *nand)
{
	return readb(NAND_CTRL_BASE + NF_STAT) & 0x1;
}


int nand_init(struct nand_chip *nand)
{
#ifdef CONFIG_S3C2410
	writel(NAND_CTRL_BASE + NF_CONF, 0x9333);
#elif defined(CONFIG_S3C2440)
	writel(NAND_CTRL_BASE + NF_CONF, 0x2440);
	writel(NAND_CTRL_BASE + NF_CONT, 0x1);
#endif

	nand->cmmd_port = NAND_CTRL_BASE + NF_CMMD;
	nand->addr_port = NAND_CTRL_BASE + NF_ADDR;
	nand->data_port = NAND_CTRL_BASE + NF_DATA;
	nand->flash_ready = s3c24x0_nand_ready;

	return nand_probe(nand);
}

