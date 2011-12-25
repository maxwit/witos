#include <stdio.h>
#include <loader.h>
#include <flash/nand.h>

// fixme
#define CONFIG_GBH_DEF_LEN   KB(512)

#define TIMEOUT_COUNT        (1 << 14)
#define IS_LARGE_PAGE(flash) (flash->write_size >= KB(1))
#define IS_POW2(n)           ((n) && ((n) & ((n) - 1)) == 0)

static const __u8 nand_ids[] = {
	0x33, 0x35, 0x36, 0x39, 0x43, 0x45, 0x46, 0x49, 0x53, 0x55,
	0x56, 0x59, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x78, 0x79,
};

// Basic operations
static void inline nand_write_cmd(struct nand_chip *flash, __u8 cmd)
{
	writeb(flash->cmmd_port, cmd);
}

static void inline nand_write_addr(struct nand_chip *flash, __u8 addr)
{
	writeb(flash->addr_port, addr);
}

#ifdef CONFIG_NAND_16BIT
static __u16 inline nand_read_data(struct nand_chip *flash)
{
	return readw(flash->data_port);
}
#else
static __u8 inline nand_read_data(struct nand_chip *flash)
{
	return readb(flash->data_port);
}
#endif

static int nand_wait_ready(struct nand_chip *flash)
{
	int i;

	if (flash->flash_ready) {
		for (i = 0; i < TIMEOUT_COUNT; i++) {
			if (flash->flash_ready(flash))
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

static void nand_send_cmd(struct nand_chip *flash,
						__u32 cmd, int page_idx, int page_off)
{
	nand_write_cmd(flash, cmd);

	if (page_off != -1) {
		nand_write_addr(flash, page_off & 0xff);

		if (IS_LARGE_PAGE(flash))
			nand_write_addr(flash, (page_off >> 8) & 0xff);
	}

	if (page_idx != -1) {
		nand_write_addr(flash, page_idx & 0xff);
		nand_write_addr(flash, (page_idx >> 8) & 0xff);

		// if ((page_idx >> 16) & 0xff)	 // fixme
			nand_write_addr(flash, (page_idx >> 16) & 0xff); //" &0xff" could be ignored
	}

	switch (cmd) {
	case NAND_CMMD_READ0:
		if (IS_LARGE_PAGE(flash))
			nand_write_cmd(flash, NAND_CMMD_READSTART);
		break;

	default:
		break;
	}

	nand_wait_ready(flash);
}

static int nand_probe(struct nand_chip *flash)
{
	__u8  dev_id, ven_id, ext_id;
	int front, end;

	nand_write_cmd(flash, NAND_CMMD_RESET);
	nand_wait_ready(flash);

	nand_send_cmd(flash, NAND_CMMD_READID, -1, 0);
	ven_id = nand_read_data(flash);
	dev_id = nand_read_data(flash);

	if (dev_id == 0) {
		printf("NAND not found!\n");
		return -1;
	}

	printf("NAND = 0x%x%x\n", ven_id, dev_id);

	front = 0;
	end = ARRAY_ELEM_NUM(nand_ids);

	while (front <= end) {
#if 0
		int nMid = (front + end) / 2;

		if (nand_ids[nMid] == dev_id) {
			flash->write_size = 512;
			flash->erase_size = KB(16);

			goto L1;
		}

		if (dev_id < nand_ids[nMid])
			end = nMid -1;
		else
			front = nMid + 1;
#else
		if (nand_ids[front] == dev_id) {
			flash->write_size = 512;
			flash->erase_size = KB(16);

			goto L1;
		}

		front++;
#endif
	}

	ext_id = nand_read_data(flash);
	ext_id = nand_read_data(flash);

	flash->write_size = KB(1) << (ext_id & 0x3);
	flash->erase_size = KB(64) << ((ext_id >> 4) & 0x03);

L1:
#ifdef CONFIG_DEBUG
	printf("(Page Size = 0x%x, Block Size = 0x%x)\n",
		flash->write_size, flash->erase_size);
#endif

	return 0;
}

static void *nand_read_page(struct nand_chip *flash, __u32 page_idx, void *buff)
{
	size_t count;
#ifdef CONFIG_NAND_16BIT
	__u16 *data;
#else
	__u8 *data;
#endif

	nand_send_cmd(flash, NAND_CMMD_READ0, page_idx, 0);

	data = buff;
	for (count = 0; count < flash->write_size / sizeof(*data); count++) {
		*data = nand_read_data(flash);
		data++;
	}

	return data;
}

// load bottom-half from nand with timeout checking
int __WEAK__ nand_load(struct nand_chip *flash, __u32 block, void *mem)
{
	__u32 curr_page, last_page;
	__u32 wshift = 0, eshift = 0, shift;
	__u32 load_size;
	void *buff;

	// yes, calc shift in this way.
	for (shift = 0; shift < WORD_SIZE * 8; shift++) {
		if ((1 << shift) == flash->write_size)
			wshift = shift;
		else if ((1 << shift) == flash->erase_size)
			eshift = shift;
	}

	if (0 == wshift || 0 == eshift)
		return -1;

	curr_page = block << (eshift - wshift);
	buff = nand_read_page(flash, curr_page, mem);

	if (GBH_MAGIC == readl(mem + GBH_MAGIC_OFFSET)) {
		load_size = readl(mem + GBH_SIZE_OFFSET);
#ifdef CONFIG_DEBUG
		printf("g-bios-bh found.\n");
#endif
	} else {
		load_size = CONFIG_GBH_DEF_LEN;
#ifdef CONFIG_DEBUG
		printf("g-bios-bh NOT found!\n");
#endif
	}

	last_page = curr_page + ((load_size - 1) >> wshift);

#ifdef CONFIG_DEBUG
	printf("Nand loader: memory = 0x%x, flash = 0x%x, size = 0x%x\n",
		mem, curr_page, load_size);
#endif

	while (++curr_page <= last_page)
		buff = nand_read_page(flash, curr_page, buff);

	return buff - mem;
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

	ret = nand_load(&nand, CONFIG_GBH_START_BLK, (void *)CONFIG_GBH_START_MEM);

	return ret;
}

REGISTER_LOADER(n, nand_loader, "NAND");
