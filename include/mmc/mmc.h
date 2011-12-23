#pragma once

#include <drive.h>

#ifdef CONFIG_GTH
int mmc_init(void);
#endif

#define MMC_GO_IDLE_STATE         0   /* bc                          */
#define MMC_SEND_OP_COND          1   /* bcr  [31:0] OCR         R3  */
#define MMC_ALL_SEND_CID          2   /* bcr                     R2  */
#define MMC_SET_RELATIVE_ADDR     3   /* ac   [31:16] RCA        R1  */
#define MMC_SET_DSR               4   /* bc   [31:16] RCA            */
#define MMC_SLEEP_AWAKE		  5   /* ac   [31:16] RCA 15:flg R1b */
#define MMC_SWITCH                6   /* ac   [31:0] See below   R1b */
#define MMC_SELECT_CARD           7   /* ac   [31:16] RCA        R1  */
#define MMC_SEND_EXT_CSD          8   /* adtc                    R1  */
#define MMC_SEND_CSD              9   /* ac   [31:16] RCA        R2  */
#define MMC_SEND_CID             10   /* ac   [31:16] RCA        R2  */
#define MMC_READ_DAT_UNTIL_STOP  11   /* adtc [31:0] dadr        R1  */
#define MMC_STOP_TRANSMISSION    12   /* ac                      R1b */
#define MMC_SEND_STATUS          13   /* ac   [31:16] RCA        R1  */
#define MMC_GO_INACTIVE_STATE    15   /* ac   [31:16] RCA            */
#define MMC_SPI_READ_OCR         58   /* spi                  spi_R3 */
#define MMC_SPI_CRC_ON_OFF       59   /* spi  [0:0] flag      spi_R1 */

  /* class 2 */
#define MMC_SET_BLOCKLEN         16   /* ac   [31:0] block len   R1  */
#define MMC_READ_SINGLE_BLOCK    17   /* adtc [31:0] data addr   R1  */
#define MMC_READ_MULTIPLE_BLOCK  18   /* adtc [31:0] data addr   R1  */

  /* class 3 */
#define MMC_WRITE_DAT_UNTIL_STOP 20   /* adtc [31:0] data addr   R1  */

  /* class 4 */
#define MMC_SET_BLOCK_COUNT      23   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_BLOCK          24   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_MULTIPLE_BLOCK 25   /* adtc                    R1  */
#define MMC_PROGRAM_CID          26   /* adtc                    R1  */
#define MMC_PROGRAM_CSD          27   /* adtc                    R1  */

  /* class 6 */
#define MMC_SET_WRITE_PROT       28   /* ac   [31:0] data addr   R1b */
#define MMC_CLR_WRITE_PROT       29   /* ac   [31:0] data addr   R1b */
#define MMC_SEND_WRITE_PROT      30   /* adtc [31:0] wpdata addr R1  */

  /* class 5 */
#define MMC_ERASE_GROUP_START    35   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE_GROUP_END      36   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE                38   /* ac                      R1b */

  /* class 9 */
#define MMC_FAST_IO              39   /* ac   <Complex>          R4  */
#define MMC_GO_IRQ_STATE         40   /* bcr                     R5  */

  /* class 7 */
#define MMC_LOCK_UNLOCK          42   /* adtc                    R1b */

  /* class 8 */
#define MMC_APP_CMD              55   /* ac   [31:16] RCA        R1  */
#define MMC_GEN_CMD              56   /* adtc [0] RD/WR          R1  */

/*sd*/
#define SD_APP_SET_BUS_WIDTH      6   /* ac   [1:0] bus width    R1  */
#define SD_SEND_IF_COND           8   /* bcr  [11:0] See below   R7  */
#define SD_APP_OP_COND           41   /* bcr  [31:0] OCR         R3  */

#define MMC_CMD_RETRIES        20
#define SD_BUS_WIDTH_1		0
#define SD_BUS_WIDTH_4		2

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

struct mmc_command {
	__u32 index;
	__u32 arg;
//	__u32 val;
	RESP resp;
};

struct sdio_cccr {
	unsigned int  sdio_vsn;
	unsigned int  sd_vsn;
	unsigned int  multi_block:1,
				  low_speed:1,
				  wide_bus:1,
				  high_power:1,
				  high_speed:1,
				  disable_cd:1;
};

struct sdio_cis {
	unsigned short		vendor;
	unsigned short		device;
	unsigned short		blksize;
	unsigned int		max_dtr;
};

struct mmc_cid {
	__u8	alw1:1;
	__u8	crc:7;
	__u16 mdt:12;
	__u16 rev:4;
	__u32 psn;
	__u8	prv;
	__u8	pnm[5];
	__u16 oid;
	__u8	mid;
}__PACKED__;

struct mmc_card {
	struct disk_drive drive;

	__u16 rca;
	__u16 dsr;
	__u32 blen;
	__u32	raw_cid[4];	/* raw card CID */
	__u32	raw_csd[4];	/* raw card CSD */
	struct sdio_cccr	cccr;		/* common card info */
	struct sdio_cis		cis;		/* common tuple info */
	char card_name[6];

	struct mmc_host *host;
};

struct mmc_host {
	__u32 resp[4];
	struct mmc_card card;

	int (*send_cmd)(struct mmc_host *mmc, __u32 index, __u32 arg, RESP resp);
	void (*set_hclk)(void);
	void (*set_lclk)(void);
	int (*read_data)(struct mmc_host *mmc, void *buf);
	int (*write_data)(struct mmc_host *mmc, const void *buf);

};

int mmc_register(struct mmc_host * mmc);

struct mmc_host * mmc_get_host(int id);

int mmc_read_blk(struct mmc_host *host, __u8 *buf, int start);

int mmc_write_blk(struct mmc_host *host, const __u8 *buf, int start);

int mmc_erase_blk(struct mmc_host *host, int start);

int mmc_decode_cid(struct mmc_host *host);

int mmc_sd_detect_card(struct mmc_host *host);
