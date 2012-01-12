#pragma once

#include <types.h>

// standard nand commands
#define NAND_CMMD_READ0         0x00
#define NAND_CMMD_READ1         0x01
#define NAND_CMMD_RNDOUT        0x05
#define NAND_CMMD_PAGEPROG      0x10
#define NAND_CMMD_READOOB       0x50
#define NAND_CMMD_ERASE1        0x60
#define NAND_CMMD_STATUS        0x70
#define NAND_CMMD_STATUS_MULTI  0x71
#define NAND_CMMD_SEQIN         0x80
#define NAND_CMMD_RNDIN         0x85
#define NAND_CMMD_READID        0x90
#define NAND_CMMD_ERASE2        0xd0
#define NAND_CMMD_RESET         0xff

#define NAND_CMMD_READSTART     0x30
#define NAND_CMMD_RNDOUTSTART   0xE0
#define NAND_CMMD_CACHEDPROG    0x15

#define NAND_CMMD_DEPLETE1      0x100
#define NAND_CMMD_DEPLETE2      0x38
#define NAND_CMMD_STATUS_MULTI  0x71
#define NAND_CMMD_STATUS_ERROR  0x72
#define NAND_CMMD_STATUS_ERROR0 0x73
#define NAND_CMMD_STATUS_ERROR1 0x74
#define NAND_CMMD_STATUS_ERROR2 0x75
#define NAND_CMMD_STATUS_ERROR3 0x76
#define NAND_CMMD_STATUS_RESET  0x7f
#define NAND_CMMD_STATUS_CLEAR  0xff

#define NAND_CMMD_NONE          -1

// Nand description
#define SOFT_ECC_DATA_LEN  256
#define SOFT_ECC_CODE_NUM  3

#define NAND_MAX_CHIPS        8
#define NAND_MAX_OOB_SIZE    64
#define NAND_MAX_PAGESIZE    2048

#define NAND_NCE        0x01
#define NAND_CLE        0x02
#define NAND_ALE        0x04

#define NAND_CTRL_CLE        (NAND_NCE | NAND_CLE)
#define NAND_CTRL_ALE        (NAND_NCE | NAND_ALE)
#define NAND_CTRL_CHANGE    0x80

#define NAND_STATUS_FAIL        0x01
#define NAND_STATUS_FAIL_N1     0x02
#define NAND_STATUS_TRUE_READY  0x20
#define NAND_STATUS_READY       0x40
#define NAND_STATUS_WP          0x80

#define NAND_ECC_READ        0
#define NAND_ECC_WRITE        1
#define NAND_ECC_READSYN    2

#define NAND_NO_AUTOINCR    0x00000001
#define NAND_BUSWIDTH_16    0x00000002
#define NAND_NO_PADDING        0x00000004
#define NAND_CACHEPRG        0x00000008
#define NAND_COPYBACK        0x00000010
#define NAND_IS_AND            0x00000020
#define NAND_4PAGE_ARRAY    0x00000040
#define BBT_AUTO_REFRESH    0x00000080
#define NAND_NO_READRDY        0x00000100
#define NAND_NO_SUBPAGE_WRITE    0x00000200

#define NAND_SAMSUNG_LP_OPTIONS \
	(NAND_NO_PADDING | NAND_CACHEPRG | NAND_COPYBACK)

#define NAND_CANAUTOINCR(nand) (!(nand->flags & NAND_NO_AUTOINCR))
#define NAND_MUST_PAD(nand) (!(nand->flags & NAND_NO_PADDING))
#define NAND_HAS_CACHEPROG(nand) ((nand->flags & NAND_CACHEPRG))
#define NAND_HAS_COPYBACK(nand) ((nand->flags & NAND_COPYBACK))

#define NAND_CHIP_OPTIONS_MSK    (0x0000ffff & ~NAND_NO_AUTOINCR)
#define NAND_USE_FLASH_BBT    0x00010000
#define NAND_SKIP_BBTSCAN    0x00020000
#define NAND_OWN_BUFFERS    0x00040000

#define LP_OPTIONS (NAND_SAMSUNG_LP_OPTIONS | NAND_NO_READRDY | NAND_NO_AUTOINCR)
#define LP_OPTIONS16 (LP_OPTIONS | NAND_BUSWIDTH_16)

struct nand_desc {
#ifdef CONFIG_GTH
	int   id;
	__u32 size;
#else
	const char *name;
	int   id;
	__u32 write_size;
	__u32 chip_size;
	__u32 erase_size;
#endif
	__u32 flags;
};

#ifdef CONFIG_GTH
#define NAND_CHIP_DESC(n, i, w, c, e, f) \
	{.id = i, .size = (c) << 20 | (w), .flags = f}
#else
#define NAND_CHIP_DESC(n, i, w, c, e, f) \
	{.name = n, .id = i, .write_size = w, .chip_size = c, .erase_size = e, .flags = f}
#endif

struct nand_chip;

#ifdef CONFIG_GTH

int nand_init(struct nand_chip *);
int nand_probe(struct nand_chip *);
void *nand_read_page(struct nand_chip *, __u32, void *);

#else

#include <flash/flash.h>

typedef enum {
	FL_READY,
	FL_READING,
	FL_WRITING,
	FL_ERASING,
	FL_SYNCING,
	FL_CACHEDPRG,
	FL_PM_SUSPENDED,
} NAND_STATE;

struct nand_ctrl;

struct nand_buffer {
	__u8 ecccalc[NAND_MAX_OOB_SIZE];
	__u8 ecccode[NAND_MAX_OOB_SIZE];
	__u8 data_buff[NAND_MAX_PAGESIZE + NAND_MAX_OOB_SIZE];
};

struct nand_bad_blk {
	int flags;
	int pages[NAND_MAX_CHIPS];
	int offs;
	int veroffs;
	int len;
	int maxblocks;
	int reserved_block_code;
	__u8  version[NAND_MAX_CHIPS];
	__u8 *pattern;
};

struct nand_ctrl {
	void  *cmmd_reg;
	void  *addr_reg;
	void  *data_reg;

	int   slaves;
	__u32   max_slaves;

	int   chip_delay;
	NAND_STATE  state;

	// private :
	__u8    (*read_byte)(struct nand_ctrl *);
	__u16   (*read_word)(struct nand_ctrl *);
	void  (*write_buff)(struct nand_ctrl *, const __u8 *, int);
	void  (*read_buff)(struct nand_ctrl *, __u8 *, int);
	int   (*verify_buff)(struct nand_ctrl *, const __u8 *, int);
	void  (*select_chip)(struct nand_chip *, bool);
	int   (*block_bad)(struct nand_chip *, __u32, int);
	int   (*block_mark_bad)(struct nand_chip *, __u32);
	void  (*cmd_ctrl)(struct nand_chip *, int, unsigned int);
	int   (*flash_ready)(struct nand_chip *);
	void  (*command)(struct nand_chip * nand, __u32 cmd, int col, int row);
	int   (*wait_func)(struct nand_chip *);
	void  (*erase_block)(struct nand_chip *, int);
	int   (*scan_bad_block)(struct nand_chip *);

	// public:
	int   (*read_page)(struct nand_chip *, __u8 *);
	void  (*write_page)(struct nand_chip *, const __u8 *);
	int   (*read_page_raw)(struct nand_chip *, __u8 *);
	void  (*write_page_raw)(struct nand_chip *, const __u8 *);
	int   (*read_oob)(struct nand_chip *, int, int);
	int   (*write_oob)(struct nand_chip *, int);

	ECC_MODE ecc_mode;

	struct nand_oob_layout *hard_oob_layout;
	struct nand_oob_layout *soft_oob_layout;
	struct nand_oob_layout *curr_oob_layout;

	__u32  ecc_data_len;
	__u32  ecc_code_len;

	void  (*ecc_enable)(struct nand_chip *nand, int mode);
	int   (*ecc_generate)(struct nand_chip *nand, const __u8 *data, __u8 *ecc);
	int   (*ecc_correct)(struct nand_chip *nand, __u8 *data, __u8 *ecc_read, __u8 *ecc_calc);

	const char *name;

	struct list_node nand_list;
};

#define NAND_MFR_TOSHIBA    0x98
#define NAND_MFR_SAMSUNG    0xec
#define NAND_MFR_FUJITSU    0x04
#define NAND_MFR_NATIONAL    0x8f
#define NAND_MFR_RENESAS    0x07
#define NAND_MFR_STMICRO    0x20
#define NAND_MFR_HYNIX        0xad
#define NAND_MFR_MICRON        0x2c
#define NAND_MFR_AMD        0x01

struct nand_vendor_name {
	int  id;
	const char *name;
};

#define NAND_BBT_NRBITS_MSK    0x0000000F
#define NAND_BBT_1BIT        0x00000001
#define NAND_BBT_2BIT        0x00000002
#define NAND_BBT_4BIT        0x00000004
#define NAND_BBT_8BIT        0x00000008

#define NAND_BBT_LASTBLOCK    0x00000010
#define NAND_BBT_ABSPAGE    0x00000020
#define NAND_BBT_SEARCH        0x00000040
#define NAND_BBT_PERCHIP    0x00000080
#define NAND_BBT_VERSION    0x00000100
#define NAND_BBT_CREATE        0x00000200
#define NAND_BBT_SCANALLPAGES    0x00000400
#define NAND_BBT_SCANEMPTY    0x00000800
#define NAND_BBT_WRITE        0x00001000
#define NAND_BBT_SAVECONTENT    0x00002000
#define NAND_BBT_SCAN2NDPAGE    0x00004000

#define NAND_BBT_SCAN_MAXBLOCKS    4

int nand_scan_bbt(struct nand_chip *nand);
int nand_update_bbt(struct nand_chip *nand, __u32 offs);
int nand_is_bad_bbt(struct nand_chip *nand, __u32 offs);
int nand_erase(struct nand_chip *nand, struct erase_opt *opt);

#define NAND_BBP_LARGE        0
#define NAND_BBP_SMALL        5

#define FLASH_TO_NAND(flash)  container_of(flash, struct nand_chip, parent)
#define NAND_TO_FLASH(nand)   (&nand->parent)

ECC_MODE nand_set_ecc_mode(struct nand_ctrl *nfc, ECC_MODE new_mode);

int nand_calculate_ecc(struct nand_chip *nand, const __u8 *data, __u8 *ecc);

int nand_correct_data(struct nand_chip *nand, __u8 *dat, __u8 *read_ecc, __u8 *calc_ecc);

#define PAGE_SIZE_AUTODETECT 0

int nand_ctrl_register(struct nand_ctrl *);

struct nand_ctrl *nand_ctrl_new(void);

struct nand_chip *nand_probe(struct nand_ctrl *nfc, int bus_idx);

int nand_register(struct nand_chip * nand);
#endif

struct nand_chip {
#ifdef CONFIG_GTH
	void *cmmd_port;
	void *addr_port;
	void *data_port;

	size_t write_size;
	size_t chip_size;

	int (*nand_ready)(struct nand_chip *);
	void *(*read_buff)(struct nand_chip *, void *, size_t);
#else
	struct flash_chip parent;
	struct nand_ctrl *master;

	__u32  device_id;
	__u32  vendor_id;

	__u32  flags;

	int  phy_erase_shift;
	int  bbt_erase_shift;

	int  page_num_mask;
	int  page_in_buff;

	int  bad_blk_oob_pos;

	struct oob_opt opt;

	__u8 *bbt;
	__u8 *oob_buf;

	struct nand_buffer *buffers;

	struct nand_bad_blk *bbt_td;
	struct nand_bad_blk *bbt_md;
	struct nand_bad_blk *bad_blk_patt;

	__u32 bus_idx;
	const char *name; // id or just name?

	struct list_node nand_node;
#endif
};
