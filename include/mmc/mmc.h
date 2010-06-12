#pragma once

#define MMC_CMD0   0
#define MMC_CMD1   1
#define MMC_CMD2   2
#define MMC_CMD3   3
#define MMC_CMD4   4
#define MMC_CMD5   5
#define MMC_CMD6   6
#define MMC_CMD7   7
#define MMC_CMD8   8
#define MMC_CMD9   9
#define MMC_CMD10  10
#define MMC_CMD11  11
#define MMC_CMD12  12
#define MMC_CMD13  13
#define MMC_CMD14  14
#define MMC_CMD15  15
#define MMC_CMD16  16
#define MMC_CMD17  17
#define MMC_CMD18  18
#define MMC_CMD19  19
#define MMC_CMD20  20
#define MMC_CMD21  21
#define MMC_CMD22  22
#define MMC_CMD23  23
#define MMC_CMD24  24
#define MMC_CMD25  25
#define MMC_CMD26  26
#define MMC_CMD27  27
#define MMC_CMD28  28
#define MMC_CMD29  29
#define MMC_CMD30  30
#define MMC_CMD31  31
#define MMC_CMD32  32
#define MMC_CMD33  33
#define MMC_CMD34  34
#define MMC_CMD35  35
#define MMC_CMD36  36
#define MMC_CMD37  37
#define MMC_CMD38  38
#define MMC_CMD39  39
#define MMC_CMD40  40
#define MMC_CMD41  41
#define MMC_CMD42  42
#define MMC_CMD43  43
#define MMC_CMD44  44
#define MMC_CMD45  45
#define MMC_CMD46  46
#define MMC_CMD47  47
#define MMC_CMD48  48
#define MMC_CMD49  49
#define MMC_CMD50  50
#define MMC_CMD51  51
#define MMC_CMD52  52
#define MMC_CMD53  53
#define MMC_CMD54  54
#define MMC_CMD55  55
#define MMC_CMD56  56
#define MMC_CMD57  57
#define MMC_CMD58  58
#define MMC_CMD59  59
#define MMC_CMD60  60
#define MMC_CMD61  61
#define MMC_CMD62  62
#define MMC_CMD63  63
#define MMC_CMD64  64
#define MMC_CMD65  65

typedef enum{
	REV,
	NONE,
	R1,
	R1b,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
} RESP;

struct mmc_command
{
	u32 index;
	u32 arg;
//	u32 val;
	RESP resp;
};

struct sdio_cccr
{
	unsigned int  sdio_vsn;
	unsigned int  sd_vsn;
	unsigned int  multi_block:1,
				  low_speed:1,
				  wide_bus:1,
				  high_power:1,
				  high_speed:1,
				  disable_cd:1;
};

struct sdio_cis
{
	unsigned short		vendor;
	unsigned short		device;
	unsigned short		blksize;
	unsigned int		max_dtr;
};

struct mmc_card
{
	u16 rca;
	u16 dsr;
	u32 blen;
	u32	raw_cid[4];	/* raw card CID */
	u32	raw_csd[4];	/* raw card CSD */
	struct sdio_cccr	cccr;		/* common card info */
	struct sdio_cis		cis;		/* common tuple info */

};

struct mmc_host
{
	u32 resp[4];
	struct mmc_card info;

	int (*send_cmd)(struct mmc_host *mmc, u32 index, u32 arg, RESP resp);
	void (*set_hclk)(void);

	int (*read_data)(struct mmc_host *mmc, void *buf);
	int (*write_data)(struct mmc_host *mmc, void *buf);
};

int mmc_register(struct mmc_host * mmc);

