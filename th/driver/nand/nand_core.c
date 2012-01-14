#include <io.h>
#include <stdio.h>
#include <delay.h>
#include <flash/nand.h>

// #define CONFIG_NAND_DEBUG
#define NAND_TIMEOUT         (1 << 14)
#define IS_LARGE_PAGE(nand)  (nand->write_size >= KB(1))

static const struct nand_desc g_nand_chip_desc[] = {
    NAND_CHIP_DESC("NAND 16MB 1.8V 8-bit",   0x33, 512, 16, 0x4000, 0),
    NAND_CHIP_DESC("NAND 16MB 3.3V 8-bit",   0x73, 512, 16, 0x4000, 0),
    NAND_CHIP_DESC("NAND 16MB 1.8V 16-bit",  0x43, 512, 16, 0x4000, NAND_BUSWIDTH_16),
    NAND_CHIP_DESC("NAND 16MB 3.3V 16-bit",  0x53, 512, 16, 0x4000, NAND_BUSWIDTH_16),

    NAND_CHIP_DESC("NAND 32MB 1.8V 8-bit",   0x35, 512, 32, 0x4000, 0),
    NAND_CHIP_DESC("NAND 32MB 3.3V 8-bit",   0x75, 512, 32, 0x4000, 0),
    NAND_CHIP_DESC("NAND 32MB 1.8V 16-bit",  0x45, 512, 32, 0x4000, NAND_BUSWIDTH_16),
    NAND_CHIP_DESC("NAND 32MB 3.3V 16-bit",  0x55, 512, 32, 0x4000, NAND_BUSWIDTH_16),

    NAND_CHIP_DESC("NAND 64MB 1.8V 8-bit",   0x36, 512, 64, 0x4000, 0),
    NAND_CHIP_DESC("NAND 64MB 3.3V 8-bit",   0x76, 512, 64, 0x4000, 0),
    NAND_CHIP_DESC("NAND 64MB 1.8V 16-bit",  0x46, 512, 64, 0x4000, NAND_BUSWIDTH_16),
    NAND_CHIP_DESC("NAND 64MB 3.3V 16-bit",  0x56, 512, 64, 0x4000, NAND_BUSWIDTH_16),

    NAND_CHIP_DESC("NAND 128MB 1.8V 8-bit",  0x78, 512, 128, 0x4000, 0),
    NAND_CHIP_DESC("NAND 128MB 1.8V 8-bit",  0x39, 512, 128, 0x4000, 0),
    NAND_CHIP_DESC("NAND 128MB 3.3V 8-bit",  0x79, 512, 128, 0x4000, 0),
    NAND_CHIP_DESC("NAND 128MB 1.8V 16-bit", 0x72, 512, 128, 0x4000, NAND_BUSWIDTH_16),
    NAND_CHIP_DESC("NAND 128MB 1.8V 16-bit", 0x49, 512, 128, 0x4000, NAND_BUSWIDTH_16),
    NAND_CHIP_DESC("NAND 128MB 3.3V 16-bit", 0x74, 512, 128, 0x4000, NAND_BUSWIDTH_16),
    NAND_CHIP_DESC("NAND 128MB 3.3V 16-bit", 0x59, 512, 128, 0x4000, NAND_BUSWIDTH_16),

    NAND_CHIP_DESC("NAND 256MB 3.3V 8-bit",   0x71, 512, 256, 0x4000, 0),

    NAND_CHIP_DESC("NAND 64MB 1.8V 8-bit",   0xA2, 0,  64, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 64MB 3.3V 8-bit",   0xF2, 0,  64, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 64MB 1.8V 16-bit",  0xB2, 0,  64, 0, LP_OPTIONS16),
    NAND_CHIP_DESC("NAND 64MB 3.3V 16-bit",  0xC2, 0,  64, 0, LP_OPTIONS16),

    NAND_CHIP_DESC("NAND 128MB 1.8V 8-bit",  0xA1, 0, 128, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 128MB 3.3V 8-bit",  0xF1, 0, 128, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 128MB 1.8V 16-bit", 0xB1, 0, 128, 0, LP_OPTIONS16),
    NAND_CHIP_DESC("NAND 128MB 3.3V 16-bit", 0xC1, 0, 128, 0, LP_OPTIONS16),

    NAND_CHIP_DESC("NAND 256MB 1.8V 8-bit",  0xAA, 0, 256, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 256MB 3.3V 8-bit",  0xDA, 0, 256, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 256MB 1.8V 16-bit", 0xBA, 0, 256, 0, LP_OPTIONS16),
    NAND_CHIP_DESC("NAND 256MB 3.3V 16-bit", 0xCA, 0, 256, 0, LP_OPTIONS16),

    NAND_CHIP_DESC("NAND 512MB 1.8V 8-bit",  0xAC, 0, 512, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 512MB 3.3V 8-bit",  0xDC, 0, 512, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 512MB 1.8V 16-bit", 0xBC, 0, 512, 0, LP_OPTIONS16),
    NAND_CHIP_DESC("NAND 512MB 3.3V 16-bit", 0xCC, 0, 512, 0, LP_OPTIONS16),

    NAND_CHIP_DESC("NAND 1GiB 1.8V 8-bit",   0xA3, 0, 1024, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 1GiB 3.3V 8-bit",   0xD3, 0, 1024, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 1GiB 1.8V 16-bit",  0xB3, 0, 1024, 0, LP_OPTIONS16),
    NAND_CHIP_DESC("NAND 1GiB 3.3V 16-bit",  0xC3, 0, 1024, 0, LP_OPTIONS16),

    NAND_CHIP_DESC("NAND 2GiB 1.8V 8-bit",   0xA5, 0, 2048, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 2GiB 3.3V 8-bit",   0xD5, 0, 2048, 0, LP_OPTIONS),
    NAND_CHIP_DESC("NAND 2GiB 1.8V 16-bit",  0xB5, 0, 2048, 0, LP_OPTIONS16),
    NAND_CHIP_DESC("NAND 2GiB 3.3V 16-bit",  0xC5, 0, 2048, 0, LP_OPTIONS16),
};

static void nand_cmd_ctrl(struct nand_chip *nand, __u8 arg, __u32 ctrl)
{
	if (ctrl & NAND_CLE)
		writeb(nand->cmmd_port, arg);
	else // if (ctrl & NAND_ALE)
		writeb(nand->addr_port, arg);
}

static void *nand_read_buff8(struct nand_chip *nand, void *buff, size_t size)
{
	__u8 *data;

	for (data = buff; data < (__u8 *)(buff + size); data++)
		*data = readb(nand->data_port);

	return data;
}

static void *nand_read_buff16(struct nand_chip *nand, void *buff, size_t size)
{
	__u16 *data;

	for (data = buff; data < (__u16 *)(buff + size); data++)
		*data = readw(nand->data_port);

	return data;
}

static int nand_wait_ready(struct nand_chip *nand)
{
	int i;

	if (nand->nand_ready) {
		for (i = 0; i < NAND_TIMEOUT; i++) {
			if (nand->nand_ready(nand))
				return i;
		}

#ifdef CONFIG_NAND_DEBUG
		printf("Nand Timeout!\n");
#endif
		return -1;
	}

	udelay(0x500);

	return 0;
}

static void nand_command(struct nand_chip *nand,
						__u32 cmd, int row, int col)
{
	nand_cmd_ctrl(nand, cmd, NAND_CLE);

	if (col != -1) {
		nand_cmd_ctrl(nand, col & 0xff, NAND_ALE);

		if (IS_LARGE_PAGE(nand))
			nand_cmd_ctrl(nand, (col >> 8) & 0xff, NAND_ALE);
	}

	if (row != -1) {
		nand_cmd_ctrl(nand, row & 0xff, NAND_ALE);
		nand_cmd_ctrl(nand, (row >> 8) & 0xff, NAND_ALE);

		if (nand->chip_size > (1 << 27))
			nand_cmd_ctrl(nand, (row >> 16) & 0xff, NAND_ALE);
	}

	switch (cmd) {
	case NAND_CMMD_READ0:
		if (IS_LARGE_PAGE(nand))
			nand_cmd_ctrl(nand, NAND_CMMD_READSTART, NAND_CLE);

	default:
		break;
	}

	nand_wait_ready(nand); // fixme: not need for READID
}

static inline void config_nand(struct nand_chip *nand,
						const struct nand_desc *desc)
{
	__u8 ext_id;

	nand->chip_size  = desc->size & 0xFFF00000;
	nand->write_size = desc->size & 0x000FFFFF;

	if (!nand->write_size) {
		ext_id = readb(nand->data_port);
		ext_id = readb(nand->data_port);

		nand->write_size = KB(1) << (ext_id & 0x3);
	}

#ifdef CONFIG_NAND_DEBUG
	printf("NAND page size = 0x%x, chip size = 0x%x\n",
		nand->write_size, nand->chip_size);
#endif

	if (!nand->read_buff) {
		if (desc->flags & NAND_BUSWIDTH_16)
			nand->read_buff = nand_read_buff16;
		else
			nand->read_buff = nand_read_buff8;
	}
}

int nand_probe(struct nand_chip *nand)
{
	int i;
	__u8 dev_id, ven_id;

	nand_command(nand, NAND_CMMD_RESET, -1, -1);

	nand_command(nand, NAND_CMMD_READID, -1, 0);
	ven_id = readb(nand->data_port);
	dev_id = readb(nand->data_port);
	printf("NAND ID = 0x%x%x\n", ven_id, dev_id);

	for (i = 0; i < ARRAY_ELEM_NUM(g_nand_chip_desc); i++) {
		if (g_nand_chip_desc[i].id == dev_id) {
			config_nand(nand, &g_nand_chip_desc[i]);
			return 0;
		}
	}

	printf("NAND not found!\n");
	return -1;
}

#if 1
void *nand_read_page(struct nand_chip *nand, __u32 page, void *buff)
{
	nand_command(nand, NAND_CMMD_READ0, page, 0);
	return nand->read_buff(nand, buff, nand->write_size);
}
#else
int nand_read(struct nand_chip *nand, __u8 *buff, size_t size, loff_t offset)
{
	int wshift;
	int cur, end;

	if (offset & (nand->write_size - 1)) {
		GEN_DBG("invalid start address 0x%x\n", offset);
		return -1;
	}

	for (wshift = 0; wshift < WORD_BITS; wshift++) {
		if ((1 << wshift) == nand->write_size)
			break;
	}

	cur = offset >> wshift;
	end = (offset + size - 1) >> wshift;

	while (cur <= end) {
		nand_command(nand, NAND_CMMD_READ0, cur, 0);
		nand->read_buff(nand, buff, nand->write_size);

		buff += nand->write_size;
		cur++;
	}

	return 0;
}
#endif
