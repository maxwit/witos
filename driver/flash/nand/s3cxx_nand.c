#include <g-bios.h>
#include <flash/flash.h>
#include <flash/nand.h>
#include <arm/s3cxx.h>

static struct nand_oob_layout g_s3cxx_oob16_layout =
{
#if 0
#ifdef CONFIG_S3C2410
	.ecc_code_len = 3,
	.ecc_pos      = {0, 1, 2},
	.free_region  = {{8, 8}}
#endif

#ifdef CONFIG_S3C2440
	.ecc_code_len = 3,
	.ecc_pos    = {0, 1, 2},
	.free_region = {{8, 8}}
#endif

#ifdef CONFIG_S3C6410
	.ecc_code_len = 3,
	.ecc_pos    = {0, 1, 2},
	.free_region = {{8, 8}}
#endif
#else
	.ecc_code_len = 3,
	.ecc_pos    = {0, 1, 2},
	.free_region = {{8, 8}}
#endif
};

static struct nand_oob_layout g_s3cxx_oob64_layout =
{
#if 0
#ifdef CONFIG_S3C2410
#endif

#ifdef CONFIG_S3C2440
	.ecc_code_len = 24,
	.ecc_pos =
	{
	   40, 41, 42, 43, 44, 45, 46, 47,
	   48, 49, 50, 51, 52, 53, 54, 55,
	   56, 57, 58, 59, 60, 61, 62, 63
	},
	.free_region = {{2, 38}}
#endif

#ifdef CONFIG_S3C6410
	.ecc_code_len = 24,
	.ecc_pos =
	{
	   40, 41, 42, 43, 44, 45, 46, 47,
	   48, 49, 50, 51, 52, 53, 54, 55,
	   56, 57, 58, 59, 60, 61, 62, 63
	},
	.free_region = {{2, 38}}
#endif
#else
	.ecc_code_len = 24,
	.ecc_pos =
	{
	   40, 41, 42, 43, 44, 45, 46, 47,
	   48, 49, 50, 51, 52, 53, 54, 55,
	   56, 57, 58, 59, 60, 61, 62, 63
	},
	.free_region = {{2, 38}}
#endif
};

static int s3cxx_nand_is_ready(struct nand_chip *nand);
static void  s3cxx_nand_read_buff(struct nand_ctrl *nfc, u8 *buff, int size);
static void  s3cxx_nand_write_buff(struct nand_ctrl *nfc, const u8 *buff, int size);
static void s3cxx_nand_enable_hwecc(struct nand_chip *nand, int mode);
static int s3cxx_nand_calc_hwecc(struct nand_chip *nand, const u8 *data, u8 *ecc);
static int s3cxx_nand_correct_data(struct nand_chip *nand,
								u8 *data, u8 *ecc_read, u8 *ecc_calc);

static int s3cxx_nand_init(struct nand_ctrl *nfc);
static int s3cxx_nand_init(struct nand_ctrl *nfc);
static int __INIT__ s3cxx_nand_probe(void);


static int s3cxx_nand_is_ready(struct nand_chip *nand)
{
	return readb(VA(NAND_CTRL_BASE + NF_STAT)) & 0x01;
}


static void  s3cxx_nand_read_buff(struct nand_ctrl *nfc, u8 *buff, int size)
{
	while (size > 3)
	{
		*(u32 *)buff = readl(VA(NAND_CTRL_BASE + NF_DATA));

		buff += 4;
		size -= 4;
	}

	while (size > 0)
	{
		*buff = readb(VA(NAND_CTRL_BASE + NF_DATA));

		buff++;
		size--;
	}
}


static void  s3cxx_nand_write_buff(struct nand_ctrl *nfc, const u8 *buff, int size)
{
	while (size > 3)
	{
		writel(VA(NAND_CTRL_BASE + NF_DATA), *(u32 *)buff);

		buff += 4;
		size -= 4;
	}

	while (size > 0)
	{
		writeb(VA(NAND_CTRL_BASE + NF_DATA), *buff);

		buff++;
		size--;
	}
}


static void s3cxx_nand_enable_hwecc(struct nand_chip *nand, int mode)
{
#ifdef CONFIG_S3C2410
	u32 ctrl;

	ctrl = readl((void *)(NAND_CTRL_BASE + NF_CONF));
	ctrl |= 1 << 12;
	writel((void *)(NAND_CTRL_BASE + NF_CONF), ctrl);
#endif

#if defined CONFIG_S3C2440 ||CONFIG_S3C6410
	u32 val;

	val = readl(VA(NAND_CTRL_BASE + NF_CONT));
	val |= 1 << 4;
	writel(VA(NAND_CTRL_BASE + NF_CONT), val);
#endif
}


static int s3cxx_nand_calc_hwecc(struct nand_chip *nand, const u8 *data, u8 *ecc)
{
#ifdef CONFIG_S3C2410
	ecc[0] = readb(VA(NAND_CTRL_BASE + NF_ECC + 0));
	ecc[1] = readb(VA(NAND_CTRL_BASE + NF_ECC + 1));
	ecc[2] = readb(VA(NAND_CTRL_BASE + NF_ECC + 2));
#endif

#ifdef CONFIG_S3C2440
	u32 val = readl(VA(NAND_CTRL_BASE + NF_ECC0));

	ecc[0] = val & 0xFF;
	ecc[1] = (val >> 8) & 0xFF;
	ecc[2] = (val >> 16) & 0xFF;
#endif

#ifdef CONFIG_S3C6410

#endif

	return 0;
}


static int s3cxx_nand_correct_data(struct nand_chip *nand,
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

	return -EIO;
}


static int s3cxx_nand_init(struct nand_ctrl *nfc)
{
#ifdef CONFIG_S3C2410
/*init,enanle
	u32 stat;

	writel(VA(NAND_CTRL_BASE + NF_CONF), (1 << 12) | (3 << 8) | (3 << 4) | 3);
	stat = readl(VA(NAND_CTRL_BASE + NF_CONF));
	stat |= 1 << 15;
	writel(VA(NAND_CTRL_BASE + NF_CONF), stat);
	*/
	writel(VA(NAND_CTRL_BASE + NF_CONF), 0x9333);
#endif

#ifdef CONFIG_S3C2412

#endif

#ifdef CONFIG_S3C2440
	writel(VA(NAND_CTRL_BASE + NF_CONF), 0x2440);
	writel(VA(NAND_CTRL_BASE + NF_CONT), 0x1);
#endif

#ifdef CONFIG_S3C6410
	writel(VA(NAND_CTRL_BASE+ NF_CONF), 0 << 12 | 2 << 8 | 1 << 4 | 0x4);
	writel(VA(NAND_CTRL_BASE + NF_CONT), 0x1);
#endif

	return 0;
}

static int __INIT__ s3cxx_nand_probe(void)
{
	int i, ret;
	u32 stat;
	struct nand_ctrl *nfc;
	struct nand_chip *nand;

	nfc = nand_ctrl_new();
	if (NULL == nfc)
	{
		return -ENOMEM;
	}

#ifdef CONFIG_S3C2410
	strcpy(nfc->name, "s3c2410_nand");
#endif

#ifdef CONFIG_S3C2440
	strcpy(nfc->name, "s3c2440_nand");
#endif

#ifdef CONFIG_S3C6410
	strcpy(nfc->name, "s3c6410_nand");
#endif


	nfc->cmmd_reg = VA(NAND_CTRL_BASE + NF_CMMD);
	nfc->addr_reg = VA(NAND_CTRL_BASE + NF_ADDR);
	nfc->data_reg = VA(NAND_CTRL_BASE + NF_DATA);

	nfc->ecc_mode = CONFIG_NAND_ECC_MODE;

	nfc->flash_ready  = s3cxx_nand_is_ready;
	nfc->read_buff    = s3cxx_nand_read_buff;
	nfc->write_buff   = s3cxx_nand_write_buff;
	// ECC
	nfc->ecc_enable   = s3cxx_nand_enable_hwecc;
	nfc->ecc_generate = s3cxx_nand_calc_hwecc;
	nfc->ecc_correct  = s3cxx_nand_correct_data;

	s3cxx_nand_init(nfc);

	for (i = 0; i < nfc->max_slaves; i++) //
	{
		nand = nand_probe(nfc, i);
		if (NULL == nand)
			break; //: or conitnue

		if (NAND_TO_FLASH(nand)->write_size < KB(2))
		{
			nfc->hard_oob_layout = &g_s3cxx_oob16_layout;
			nfc->ecc_data_len    = 512;
		}
		else
		{
			nfc->hard_oob_layout = &g_s3cxx_oob64_layout;
			nfc->ecc_data_len    = 256;
		}
		nfc->ecc_code_len = 3;

		ret = nand_register(nand);
		if (ret < 0)
		{
			goto L1;
		}
	}

	return 0;
L1:

#ifdef CONFIG_S3C2410
	//disable nand
	stat = readl((void *)(NAND_CTRL_BASE + NF_CONF));
	stat &= ~(1 << 15);
	writel((void *)(NAND_CTRL_BASE + NF_CONF), stat);
#endif
	free(nfc);
	return ret;
}

#ifdef CONFIG_S3C2410
#endif

#ifdef CONFIG_S3C2440
#endif

#ifdef CONFIG_S3C6410
#endif

DRIVER_INIT(s3cxx_nand_probe);
