#include <stdio.h>
#include <io.h>
#include <delay.h>
#include <loader.h>
#include <flash/nand.h>

#define NAND_TIMEOUT         (1 << 14)
#define IS_LARGE_PAGE(nand)  (nand->write_size >= KB(1))

static const struct nand_chip_desc g_nand_chip_desc[] = {
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

// Basic operations
static void inline nand_write_cmd(struct nand_chip *nand, __u8 cmd)
{
	writeb(nand->cmmd_port, cmd);
}

static void inline nand_write_addr(struct nand_chip *nand, __u8 addr)
{
	writeb(nand->addr_port, addr);
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

#ifdef CONFIG_DEBUG
		printf("Nand Timeout!\n");
#endif
		return -1;
	}

	udelay(0x500);

	return 0;
}

static void nand_send_cmd(struct nand_chip *nand,
						__u32 cmd, int row, int col)
{
	nand_write_cmd(nand, cmd);

	if (col != -1) {
		nand_write_addr(nand, col & 0xff);

		if (IS_LARGE_PAGE(nand))
			nand_write_addr(nand, (col >> 8) & 0xff);
	}

	if (row != -1) {
		nand_write_addr(nand, row & 0xff);
		nand_write_addr(nand, (row >> 8) & 0xff);

		if (nand->chip_size > (1 << 27))
			nand_write_addr(nand, (row >> 16) & 0xff);
	}

	switch (cmd) {
	case NAND_CMMD_READ0:
		if (IS_LARGE_PAGE(nand))
			nand_write_cmd(nand, NAND_CMMD_READSTART);
		break;

	default:
		break;
	}

	nand_wait_ready(nand);
}

static int nand_probe(struct nand_chip *nand)
{
	int i;
	__u8 dev_id, ven_id, ext_id;

	nand_send_cmd(nand, NAND_CMMD_RESET, -1, -1);

	nand_send_cmd(nand, NAND_CMMD_READID, -1, 0);
	ven_id = readb(nand->data_port);
	dev_id = readb(nand->data_port);
	printf("NAND ID = 0x%x%x\n", ven_id, dev_id);

	for (i = 0; i < ARRAY_ELEM_NUM(g_nand_chip_desc); i++) {
		if (g_nand_chip_desc[i].id == dev_id)
			goto found;
	}

	printf("NAND not found!\n");
	return -1;

found:
	nand->chip_size  = g_nand_chip_desc[i].size & 0xFFF00000;
	nand->write_size = g_nand_chip_desc[i].size & 0x000FFFFF;

	if (!nand->write_size) {
		ext_id = readb(nand->data_port);
		ext_id = readb(nand->data_port);

		nand->write_size = KB(1) << (ext_id & 0x3);
	}

#ifdef CONFIG_DEBUG
	printf("(Nand page size = 0x%x, chip size = 0x%x)\n",
		nand->write_size, nand->chip_size);
#endif

	if (!nand->read_buff) {
		if (g_nand_chip_desc[i].flags & NAND_BUSWIDTH_16)
			nand->read_buff = nand_read_buff16;
		else
			nand->read_buff = nand_read_buff8;
	}

	return 0;
}

static void *nand_read_page(struct nand_chip *nand, __u32 page, void *buff)
{
	nand_send_cmd(nand, NAND_CMMD_READ0, page, 0);
	return nand->read_buff(nand, buff, nand->write_size);
}

// load bottom-half from nand with timeout checking
int __WEAK__ nand_load(struct nand_chip *nand, __u32 nstart, void *mstart)
{
	__u32 curr_page, last_page;
	__u32 wshift;
	__u32 load_size;
	void *buff;

	for (wshift = 0; wshift < WORD_BITS; wshift++) {
		if ((1 << wshift) == nand->write_size)
			break;
	}

	if (WORD_BITS == wshift)
		return -1;

	curr_page = nstart >> wshift;

	buff = nand_read_page(nand, curr_page, mstart);

	if (GBH_MAGIC == readl(VA(mstart + GBH_MAGIC_OFFSET))) {
		load_size = readl(VA(mstart + GBH_SIZE_OFFSET));
#ifdef CONFIG_DEBUG
		printf("g-bios-bh found.\n");
#endif
	} else {
		load_size = CONFIG_GBH_LOAD_SIZE;
#ifdef CONFIG_DEBUG
		printf("g-bios-bh NOT found!\n");
#endif
	}

	last_page = curr_page + ((load_size - 1) >> wshift);

#ifdef CONFIG_DEBUG
	printf("Nand loader: memory = 0x%x, nand = 0x%x, size = 0x%x\n",
		mstart, nstart, load_size);
#endif

	while (++curr_page <= last_page)
		buff = nand_read_page(nand, curr_page, buff);

	return buff - mstart;
}

static int nand_loader(struct loader_opt *opt)
{
	int ret;
	struct nand_chip nand = {0}; // nand_chip must be initialized

	ret = nand_init(&nand);
	if (ret < 0)
		return ret;

	ret = nand_probe(&nand);
	if (ret < 0)
		return ret;

	ret = nand_load(&nand, CONFIG_GBH_START_BLK * 0x20000, VA(CONFIG_GBH_START_MEM));

	return ret;
}

REGISTER_LOADER(n, nand_loader, "NAND");
