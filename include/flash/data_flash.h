#pragma once
#include <flash/flash.h>

// -------- PDC_PTCR : (PDC Offset: 0x20) PDC Transfer Control Register --------
#define AT91C_PDC_RXTEN           (0x1 <<  0) // (PDC) Receiver Transfer Enable
#define AT91C_PDC_RXTDIS          (0x1 <<  1) // (PDC) Receiver Transfer Disable
#define AT91C_PDC_TXTEN           (0x1 <<  8) // (PDC) Transmitter Transfer Enable
#define AT91C_PDC_TXTDIS          (0x1 <<  9) // (PDC) Transmitter Transfer Disable

// -------- SPI_SR : (SPI Offset: 0x10) Status Register --------
#define AT91C_SPI_RDRF            (0x1 <<  0) // (SPI) Receive Data Register Full
#define AT91C_SPI_TDRE            (0x1 <<  1) // (SPI) Transmit Data Register Empty
#define AT91C_SPI_MODF            (0x1 <<  2) // (SPI) Mode Fault Error
#define AT91C_SPI_OVRES           (0x1 <<  3) // (SPI) Overrun Error Status
#define AT91C_SPI_ENDRX           (0x1 <<  4) // (SPI) End of Receiver Transfer
#define AT91C_SPI_ENDTX           (0x1 <<  5) // (SPI) End of Receiver Transfer
#define AT91C_SPI_RXBUFF          (0x1 <<  6) // (SPI) RXBUFF Interrupt
#define AT91C_SPI_TXBUFE          (0x1 <<  7) // (SPI) TXBUFE Interrupt
#define AT91C_SPI_NSSR            (0x1 <<  8) // (SPI) NSSR Interrupt
#define AT91C_SPI_TXEMPTY         (0x1 <<  9) // (SPI) TXEMPTY Interrupt
#define AT91C_SPI_SPIENS          (0x1 << 16) // (SPI) Enable Status

// -------- SPI_MR : (SPI Offset: 0x4) SPI Mode Register --------
#define AT91C_SPI_MSTR            (0x1 <<  0) // (SPI) Master/Slave Mode
#define AT91C_SPI_PS              (0x1 <<  1) // (SPI) Peripheral Select
#define 	AT91C_SPI_PS_FIXED                (0x0 <<  1) // (SPI) Fixed Peripheral Select
#define 	AT91C_SPI_PS_VARIABLE             (0x1 <<  1) // (SPI) Variable Peripheral Select
#define AT91C_SPI_PCSDEC          (0x1 <<  2) // (SPI) Chip Select Decode
#define AT91C_SPI_FDIV            (0x1 <<  3) // (SPI) Clock Selection
#define AT91C_SPI_MODFDIS         (0x1 <<  4) // (SPI) Mode Fault Detection
#define AT91C_SPI_LLB             (0x1 <<  7) // (SPI) Clock Selection
#define AT91C_SPI_PCS             (0xF << 16) // (SPI) Peripheral Chip Select
#define AT91C_SPI_DLYBCS          (0xFF << 24) // (SPI) Delay Between Chip Selects

// -------- SPI_CR : (SPI Offset: 0x0) SPI Control Register --------
#define AT91C_SPI_SPIEN           (0x1 <<  0) // (SPI) SPI Enable
#define AT91C_SPI_SPIDIS          (0x1 <<  1) // (SPI) SPI Disable
#define AT91C_SPI_SWRST           (0x1 <<  7) // (SPI) SPI Software reset
#define AT91C_SPI_LASTXFER        (0x1 << 24) // (SPI) SPI Last Transfer

#define   AT91C_SPI_PCS0	(0xE << 16)
#define   AT91C_SPI_PCS1	(0xD << 16)
#define   AT91C_SPI_PCS2	(0xB << 16)
#define   AT91C_SPI_PCS3	(0x7 << 16)

#define AT91C_SPI_PCS0_SERIAL_DATAFLASH		AT91C_SPI_PCS0
#define AT91C_SPI_PCS3_DATAFLASH_CARD		AT91C_SPI_PCS3

#define OP_READ_CONTINUOUS	0xE8
#define OP_READ_PAGE		0xD2

#define OP_READ_STATUS		0xD7

#define OP_READ_BUFFER1		0xD4
#define OP_READ_BUFFER2		0xD6
#define OP_WRITE_BUFFER1	0x84
#define OP_WRITE_BUFFER2	0x87

#define OP_ERASE_PAGE		0x81
#define OP_ERASE_BLOCK		0x50

#define OP_TRANSFER_BUF1	0x53
#define OP_TRANSFER_BUF2	0x55
#define OP_MREAD_BUFFER1	0xD4
#define OP_MREAD_BUFFER2	0xD6
#define OP_MWERASE_BUFFER1	0x83
#define OP_MWERASE_BUFFER2	0x86
#define OP_MWRITE_BUFFER1	0x88
#define OP_MWRITE_BUFFER2	0x89

#define OP_PROGRAM_VIA_BUF1	0x82
#define OP_PROGRAM_VIA_BUF2	0x85

#define OP_COMPARE_BUF1		0x60
#define OP_COMPARE_BUF2		0x61

#define OP_REWRITE_VIA_BUF1	0x58
#define OP_REWRITE_VIA_BUF2	0x59

/* newer chips report JEDEC manufacturer and device IDs; chip
 * serial number and OTP bits; and per-sector writeprotect.
 */
#define OP_READ_ID			0x9F
#define OP_READ_SECURITY	0x77
#define OP_WRITE_SECURITY	0x9A

#define CFG_HZ          	10000
#define CFG_SPI_WRITE_TOUT	(5 * CFG_HZ)

//#define CONFIG_DATAFLASH_WRITE_VERIFY

struct DataFlashOptMsg {
	void	*pTxCmdBuf;
	void	*pRxCmdBuf;
	void	*pTxDataBuf;
	void	*pRxDataBuf;

	__u32	nTxCmdLen;
	__u32	nRxCmdLen;
	__u32  nTxDataLen;
	__u32  nRxDataLen;

	bool	hasData;
};

struct DataFlash {
	struct flash_chip parent;

	__u8	bCommand[8];
	char	name[BLOCK_DEV_NAME_LEN];

	__u32	page_size;
	__u32  nPageShift;
	__u32	block_size;
	__u32  nBlockShift;
	__u32	ulChipSelect;

	struct  DataFlashOptMsg stOprMsg;
};
