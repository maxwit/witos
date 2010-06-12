//
#include <g-bios.h>
#include <flash/flash.h>
#include <flash/nand.h>
#include <arm/s3c24x0.h>
#include <flash/nand.h>


#if defined(CONFIG_NAND_ECC_HW)
#define S3C2410_ECC_MODE NAND_ECC_HW
#elif defined(CONFIG_NAND_ECC_SW)
#define S3C2410_ECC_MODE NAND_ECC_SW
#else
#define S3C2410_ECC_MODE NAND_ECC_NONE
#endif

static struct nand_oob_layout g_s3c24x0_oob16_layout =
{
	.ecc_code_len = 3,
	.ecc_pos      = {0, 1, 2},
	.free_region  = {{8, 8}}
};

static void s3c2410_nand_enable(struct nand_ctrl *nfc)
{
	u32 stat;

	stat = readl((void *)(NAND_CTRL_BASE + NF_CONF));
	stat |= 1 << 15;
	writel((void *)(NAND_CTRL_BASE + NF_CONF), stat);
}

static void s3c2410_nand_disable(struct nand_ctrl *nfc)
{
	u32 stat;

	stat = readl((void *)(NAND_CTRL_BASE + NF_CONF));
	stat &= ~(1 << 15);
	writel((void *)(NAND_CTRL_BASE + NF_CONF), stat);
}


static int s3c2410_nand_init(struct nand_ctrl *nfc)
{
	u32 cmd = (1 << 12) | (3 << 8) | (3 << 4) | 3;

	writel((void *)(NAND_CTRL_BASE + NF_CONF), cmd);

	return 0;
}

static void s3c2410_nand_cmd(struct nand_chip *nand, int cmd, u32 ctrl)
{
	if (cmd == NAND_CMMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		writeb((void *)(NAND_CTRL_BASE + NF_CMMD), cmd);
	else
		writeb((void *)(NAND_CTRL_BASE + NF_ADDR), cmd);
}

static void s3c2410_nand_enable_ecc(struct nand_chip *nand, int mode)
{
	u32 ctrl;

	ctrl = readl((void *)(NAND_CTRL_BASE + NF_CONF));
	ctrl |= 1 << 12;
	writel((void *)(NAND_CTRL_BASE + NF_CONF), ctrl);
}


static int s3c2410_nand_ecc_calc(struct nand_chip *nand,
									const u8 *data, u8 *ecc_code)
{
	ecc_code[0] = readb((void *)(NAND_CTRL_BASE + NF_ECC + 0));
	ecc_code[1] = readb((void *)(NAND_CTRL_BASE + NF_ECC + 1));
	ecc_code[2] = readb((void *)(NAND_CTRL_BASE + NF_ECC + 2));

	return 0;
}


static int s3c2410_nand_correct(struct nand_chip *nand,
							u8 *data, u8 *ecc_read, u8 *ecc_calc)
{
	u32 diff0, diff1, diff2;

	diff0 = ecc_read[0] ^ ecc_calc[0];
	diff1 = ecc_read[1] ^ ecc_calc[1];
	diff2 = ecc_read[2] ^ ecc_calc[2];

	if (diff0 == 0 && diff1 == 0 && diff2 == 0)
	{
		return 0;
	}

	// TODO:  add data correction code here

	return -1;
}


static int s3c2410_nand_ready(struct nand_chip *nand)
{
	return readb((void *)(NAND_CTRL_BASE + NF_STAT)) & 0x01;
}


static int __INIT__ s3c2410_nand_probe(void)
{
	int ret;
	struct nand_ctrl *nfc;


	nfc = nand_ctrl_new();

	if (NULL == nfc)
	{
		return -ENOMEM;
	}

	strcpy(nfc->name, "s3c2410_nand");

	nfc->cmmd_reg = VA(NAND_CTRL_BASE + NF_CMMD);
	nfc->addr_reg = VA(NAND_CTRL_BASE + NF_ADDR);
	nfc->data_reg = VA(NAND_CTRL_BASE + NF_DATA);

	nfc->hard_oob_layout = &g_s3c24x0_oob16_layout;
	nfc->ecc_data_len    = 512;
	nfc->ecc_code_len    = 3;

	nfc->flash_ready     = s3c2410_nand_ready;
	nfc->ecc_enable      = s3c2410_nand_enable_ecc;
	nfc->ecc_generate    = s3c2410_nand_ecc_calc;
	nfc->ecc_correct     = s3c2410_nand_correct;

	nand_set_ecc_mode(nfc, S3C2410_ECC_MODE);

	s3c2410_nand_init(nfc);

	s3c2410_nand_enable(nfc);

	ret = nand_ctrl_register(nfc);
	if (ret < 0)
	{
		goto L1;
	}

	return 0;

L1:
	s3c2410_nand_disable(nfc);
	free(nfc);

	return ret;
}

DRIVER_INIT(s3c2410_nand_probe);

