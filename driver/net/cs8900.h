#pragma once

#define CS8900_IOBASE 0x19000300
#define CONFIG_CS8900_IRQ IRQ_EINT9

#define CS_DATA0    0x0000
#define CS_DATA1    0x0002
#define CS_TxCMD    0x0004
#define CS_TxLen    0x0006
#define CS_ISQ      0x0008
#define CS_PPP      0x000A
#define CS_PPD0     0x000C
#define CS_PPD1     0x000E

#define PP_IOBA     0x0020
#define PP_INTN     0x0022
#define PP_DMACH    0x0024
#define PP_DMASOF   0x0026
#define PP_DMAFC    0x0028
#define PP_DMABC    0x002A
#define PP_MBA      0x002C
#define PP_RxCFG    0x0102
#define PP_RxCTL    0x0104
#define PP_TxCFG    0x0106
#define PP_TxCMD    0x0108
#define PP_BufCFG   0x010A
#define PP_LineCTL  0x0112
#define PP_SelfCTL  0x0114
#define PP_BusCTL   0x0116
#define PP_TestCTL  0x0118
#define PP_LineST   0x0134
#define PP_BusST    0x0138

#define Rdy4TxNow (1 << 8)
