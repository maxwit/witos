//
#include <g-bios.h>
#include <flash/flash.h>
#include <flash/nand.h>
#include <arm/s3c6410.h>


#define s3c6410_nand_readb(reg) \
	readb(VA(NAND_CTRL_BASE + reg))

#define s3c6410_nand_writeb(reg, val) \
	writeb(VA(NAND_CTRL_BASE + reg), val)

#define s3c6410_nand_readl(reg) \
	readl(VA(NAND_CTRL_BASE + reg))

#define s3c6410_nand_writel(reg, val) \
	writel(VA(NAND_CTRL_BASE + reg), val)


static struct nand_oob_layout g_s3c6410_oob16_layout =
{
	.ecc_code_len = 3,
	.ecc_pos    = {0, 1, 2},
	.free_region = {{8, 8}}
};

static struct nand_oob_layout g_s3c6410_oob64_layout =
{
	.ecc_code_len = 24,
	.ecc_pos =
	{
	   40, 41, 42, 43, 44, 45, 46, 47,
	   48, 49, 50, 51, 52, 53, 54, 55,
	   56, 57, 58, 59, 60, 61, 62, 63
	},
	.free_region = {{2, 38}}
};


static int s3c6410_nand_init(struct nand_ctrl *nfc)
{
	s3c6410_nand_writel(NFCONF, 0 << 12 | 2 << 8 | 1 << 4 | 0x4);
	s3c6410_nand_writel(NFCONT, 0x1);

	return 0;
}


static int s3c6410_nand_is_ready(struct nand_chip *nand)
{
	return readb(VA(NAND_CTRL_BASE + NFSTAT)) & 0x01;
}


static void  s3c6410_nand_write_buff(struct nand_ctrl *nfc, const u8 *buff, int size)
{
	while (size > 3)
	{
		s3c6410_nand_writel(NFDATA, *(u32 *)buff);

		buff += 4;
		size -= 4;
	}

	while (size > 0)
	{
		s3c6410_nand_writeb(NFDATA, *buff);

		buff++;
		size--;
	}
}


static void  s3c6410_nand_read_buff(struct nand_ctrl *nfc, u8 *buff, int size)
{
	while (size > 3)
	{
		*(u32 *)buff = s3c6410_nand_readl(NFDATA);

		buff += 4;
		size -= 4;
	}

	while (size > 0)
	{
		*buff = s3c6410_nand_readb(NFDATA);

		buff++;
		size--;
	}
}


static void s3c6410_nand_enable_hwecc(struct nand_chip *nand, int mode)
{
	u32 val;

	val = readl(VA(NAND_CTRL_BASE + NFCONT));
	val |= 1 << 4;
	writel(VA(NAND_CTRL_BASE + NFCONT), val);
}


static int s3c6410_nand_calc_hwecc(struct nand_chip *nand, const u8 *data, u8 *ecc)
{
#if 0
	u32 val = readl(VA(NAND_CTRL_BASE + NFECC0));

	ecc[0] = val & 0xFF;
	ecc[1] = (val >> 8) & 0xFF;
	ecc[2] = (val >> 16) & 0xFF;
#endif

	return 0;
}


static int s3c6410_nand_correct_data(struct nand_chip *nand,
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


static int __INIT__ s3c6410_nand_probe(void)
{
	int i, ret;
	struct nand_ctrl *nfc;
	struct nand_chip *nand;

	nfc = nand_ctrl_new();

	if (NULL == nfc)
	{
		return -ENOMEM;
	}

	nfc->cmmd_reg = VA(NAND_CTRL_BASE + NFCMMD);
	nfc->addr_reg = VA(NAND_CTRL_BASE + NFADDR);
	nfc->data_reg = VA(NAND_CTRL_BASE + NFDATA);

	nfc->ecc_mode = CONFIG_NAND_ECC_MODE;
	strcpy(nfc->name, "s3c6410_nand");

	nfc->flash_ready  = s3c6410_nand_is_ready;
	nfc->read_buff    = s3c6410_nand_read_buff;
	nfc->write_buff   = s3c6410_nand_write_buff;
	// ECC
	nfc->ecc_enable   = s3c6410_nand_enable_hwecc;
	nfc->ecc_generate = s3c6410_nand_calc_hwecc;
	nfc->ecc_correct  = s3c6410_nand_correct_data;

	s3c6410_nand_init(nfc);

	for (i = 0; i < nfc->max_slaves; i++) //
	{
		nand = nand_probe(nfc, i);
		if (NULL == nand)
			break; //: or conitnue

		if (NAND_TO_FLASH(nand)->write_size < KB(2))
		{
			nfc->hard_oob_layout = &g_s3c6410_oob16_layout;
			nfc->ecc_data_len    = 512;
		}
		else
		{
			nfc->hard_oob_layout = &g_s3c6410_oob64_layout;
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
	free(nfc);
	return ret;
}

DRIVER_INIT(s3c6410_nand_probe);

