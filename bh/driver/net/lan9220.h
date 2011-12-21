#pragma once

#define DEV_ID_LAN9118  0x0118
#define DEV_ID_LAN9220  0x9220

#define MAC_CR          0x1
#define ADDRH           0x2
#define ADDRL           0x3
#define MII_ACC	        0x6
#define MII_DATA	    0x7

#define RX_DATA_PORT	0x00
#define TX_DATA_PORT	0x20
#define RX_STATUS_PORT	0X40
#define RX_STATUS_PEEK	0x44
#define TX_STATUS_PORT	0x48
#define TX_STATUS_PEEK	0x4c

#define ID_REV			0x50
#define IRQ_CFG			0x54

#define INT_STS			0x58
#define INT_EN			0x5c
// MAC IRQ bits
#define INT_RSFL        (1 << 3)
#define INT_TSFL        (1 << 7)
#define INT_PHY         (1 << 18)
#define INT_RXD         (1 << 20)

#define BYTE_TEST		0x64
#define FIFO_INT		0x68
#define RX_CFG			0x6c
#define TX_CFG			0x70
#define HW_CFG			0x74
#define RX_DP_CTL		0x78
#define RX_FIFO_INF		0x7c
#define TX_FIFO_INF		0x80
#define PMT_CTRL		0x84
#define GPIO_CFG		0x88
#define GPT_CFG			0x8c
#define GPT_CNT			0x90

#define WORD_SWAP		0x98
#define FREE_RUN		0x9C
#define RX_DROP			0xA0
#define MAC_CSR_CMD		0xA4
#define MAC_CSR_DATA	0xA8
#define AFC_CFG			0xAC
#define E2P_CMD			0xB0
#define E2P_DATA		0xB4

// MII registers
#define MII_REG_INT_SRC   29
#define MII_REG_INT_MASK  30
// PHY IRQ bits
#define PHY_INT_LINK      (1 << 4)
#define PHY_INT_AN        (1 << 6)
