#pragma once

// MAX_IRQ_NUM

// Memory Controller


// Interrupt Controller
#define INTCON_BASE     0x4a000000
#define SRCPNG                0x00
#define INTMOD                0x04
#define INTMASK               0x08
#define INTPNG                0x10
#define INTOFFSET             0x14
#define SUBMASK               0x1c

#define S3C6410_IRQREG(x)   ((x) + INTCON_BASE)
#define S3C6410_EINTREG(x)  ((x) + GPIO_BASE)

#define S3C6410_SRCPND	       S3C6410_IRQREG(0x000)
#define S3C6410_INTMOD	       S3C6410_IRQREG(0x004)
#define S3C6410_INTMSK	       S3C6410_IRQREG(0x008)
#define S3C6410_PRIORITY       S3C6410_IRQREG(0x00C)
#define S3C6410_INTPND	       S3C6410_IRQREG(0x010)
#define S3C6410_INTOFFSET      S3C6410_IRQREG(0x014)
#define S3C6410_SUBSRCPND      S3C6410_IRQREG(0x018)
#define S3C6410_INTSUBMSK      S3C6410_IRQREG(0x01C)

#define S3C6410_EINTMASK       S3C6410_EINTREG(0x0A4)
#define S3C6410_EINTPEND       S3C6410_EINTREG(0X0A8)

#define S3C6412_EINTMASK       S3C6410_EINTREG(0x0B4)
#define S3C6412_EINTPEND       S3C6410_EINTREG(0X0B8)


// IRQ Number
#define S3C64XX_IRQ(x) (x)
//
#define IRQ_EINT0      S3C64XX_IRQ(0)
#define IRQ_EINT1      S3C64XX_IRQ(1)
#define IRQ_EINT2      S3C64XX_IRQ(2)
#define IRQ_EINT3      S3C64XX_IRQ(3)
#define IRQ_EINT_4TO7  S3C64XX_IRQ(4)
#define IRQ_EINT_8TO23 S3C64XX_IRQ(5)
#define IRQ_RESERVED6  S3C64XX_IRQ(6)
#define IRQ_CAM        S3C64XX_IRQ(6)
#define IRQ_BATT_FLT   S3C64XX_IRQ(7)
#define IRQ_TICK       S3C64XX_IRQ(8)
#define IRQ_WDT	       S3C64XX_IRQ(9)
#define IRQ_TIMER0     S3C64XX_IRQ(10)
#define IRQ_TIMER1     S3C64XX_IRQ(11)
#define IRQ_TIMER2     S3C64XX_IRQ(12)
#define IRQ_TIMER3     S3C64XX_IRQ(13)
#define IRQ_TIMER4     S3C64XX_IRQ(14)
#define IRQ_UART2      S3C64XX_IRQ(15)
#define IRQ_LCD	       S3C64XX_IRQ(16)
#define IRQ_DMA0       S3C64XX_IRQ(17)
#define IRQ_DMA1       S3C64XX_IRQ(18)
#define IRQ_DMA2       S3C64XX_IRQ(19)
#define IRQ_DMA3       S3C64XX_IRQ(20)
#define IRQ_SDI	       S3C64XX_IRQ(21)
#define IRQ_SPI0       S3C64XX_IRQ(22)
#define IRQ_UART1      S3C64XX_IRQ(23)
#define IRQ_RESERVED24 S3C64XX_IRQ(24)
#define IRQ_NFCON      S3C64XX_IRQ(24)
#define IRQ_USBD       S3C64XX_IRQ(25)
#define IRQ_USBH       S3C64XX_IRQ(26)
#define IRQ_IIC	       S3C64XX_IRQ(27)
#define IRQ_UART0      S3C64XX_IRQ(28)
#define IRQ_SPI1       S3C64XX_IRQ(29)
#define IRQ_RTC	       S3C64XX_IRQ(30)
#define IRQ_TC_ADC     S3C64XX_IRQ(31)
//
#define IRQ_EINT4      S3C64XX_IRQ(32)
#define IRQ_EINT5      S3C64XX_IRQ(33)
#define IRQ_EINT6      S3C64XX_IRQ(34)
#define IRQ_EINT7      S3C64XX_IRQ(35)
#define IRQ_EINT8      S3C64XX_IRQ(36)
#define IRQ_EINT9      S3C64XX_IRQ(37)
#define IRQ_EINT10     S3C64XX_IRQ(38)
#define IRQ_EINT11     S3C64XX_IRQ(39)
#define IRQ_EINT12     S3C64XX_IRQ(40)
#define IRQ_EINT13     S3C64XX_IRQ(41)
#define IRQ_EINT14     S3C64XX_IRQ(42)
#define IRQ_EINT15     S3C64XX_IRQ(43)
#define IRQ_EINT16     S3C64XX_IRQ(44)
#define IRQ_EINT17     S3C64XX_IRQ(45)
#define IRQ_EINT18     S3C64XX_IRQ(46)
#define IRQ_EINT19     S3C64XX_IRQ(47)
#define IRQ_EINT20     S3C64XX_IRQ(48)
#define IRQ_EINT21     S3C64XX_IRQ(49)
#define IRQ_EINT22     S3C64XX_IRQ(50)
#define IRQ_EINT23     S3C64XX_IRQ(51)
// LCD
#define IRQ_LCD_FIFO   S3C64XX_IRQ(52)
#define IRQ_LCD_FRAME  S3C64XX_IRQ(53)
// UART
#define IRQ_UART0_RX   S3C64XX_IRQ(54)
#define IRQ_UART0_TX   S3C64XX_IRQ(55)
#define IRQ_UART0_ERR  S3C64XX_IRQ(56)
#define IRQ_UART1_RX   S3C64XX_IRQ(57)
#define IRQ_UART1_TX   S3C64XX_IRQ(58)
#define IRQ_UART1_ERR  S3C64XX_IRQ(59)
#define IRQ_UART2_RX   S3C64XX_IRQ(60)
#define IRQ_UART3_TX   S3C64XX_IRQ(61)
#define IRQ_UART2_ERR  S3C64XX_IRQ(62)
// TC & ADC
#define IRQ_TC         S3C64XX_IRQ(63)
#define IRQ_ADC        S3C64XX_IRQ(64)

#define MAX_IRQ_NUM    (IRQ_ADC + 1)

//
#define S3C6410_EXTINT_LOWLEV	 (0x00)
#define S3C6410_EXTINT_HILEV	 (0x01)
#define S3C6410_EXTINT_FALLEDGE	 (0x02)
#define S3C6410_EXTINT_RISEEDGE	 (0x04)
#define S3C6410_EXTINT_BOTHEDGE	 (0x06)

// Clock and Power Management
#define CLOCK_BASE      0x4c000000
#define LOCKTIME              0x00
#define MPLLCON               0x04
#define UPLLCON               0x08
#define CLKDIVN               0x14


// LCD
#define S3C64XX_LCD_BASE 0x4d000000
#define LCDCON1                0x00
#define LCDCON2                0x04
#define LCDCON3                0x08
#define LCDCON4                0x0c
#define LCDCON5                0x10
#define LCDSADDR1              0x14
#define LCDSADDR2              0x18
#define LCDSADDR3              0x1c
#define LCDINTPND              0x54
#define LCDSRCPND              0x58
#define LCDINTMSK              0x5c
#define TCONSEL                0x60

#define S3C6410_VIDCON0        0x00
#define S3C6410_VIDCON1        0x04
#define S3C6410_VIDCON2        0x08
#define S3C6410_VIDTCON0       0x10
#define S3C6410_VIDTCON1       0x14
#define S3C6410_VIDTCON2       0x18
#define S3C6410_WINCON0        0x20
#define S3C6410_WINCON1        0x24
#define S3C6410_WINCON2        0x28
#define S3C6410_WINCON3        0x2C
#define S3C6410_WINCON4        0x30

#define S3C6410_VIDOSD0A        0x40
#define S3C6410_VIDOSD0B        0x44
#define S3C6410_VIDOSD0C        0x48
#define S3C6410_VIDOSD1A        0x50
#define S3C6410_VIDOSD1B        0x54
#define S3C6410_VIDOSD1C        0x58

#define S3C6410_VIDOSD1D        0x5C
#define S3C6410_VIDOSD2A        0x60
#define S3C6410_VIDOSD2B        0x64
#define S3C6410_VIDOSD2C        0x68
#define S3C6410_VISOSD2D        0x6C
#define S3C6410_VIDOSD3A        0x70
#define S3C6410_VIDOSD3B        0x74
#define S3C6410_VIDOSD3C        0x78
#define S3C6410_VIDOSD4A        0x80
#define S3C6410_VIDOSD4B        0x84
#define S3C6410_VIDOSD4C        0x88
#define S3C6410_VIDW00ADD0B0    0xA0
#define S3C6410_VIDW00ADD0B1    0xA4
#define S3C6410_VIDW01ADD0B0    0xA8
#define S3C6410_VIDW01ADD0B1    0xAC
#define S3C6410_VIDW02ADD0      0xB0
#define S3C6410_VIDW03ADD0      0xB8
#define S3C6410_VIDW04ADD0      0xC0
#define S3C6410_VIDW00ADD1B0    0xD0
#define S3C6410_VIDW00ADD1B1    0xD4
#define S3C6410_VIDW01ADD1B0    0xD8
#define S3C6410_VIDW01ADD1B1    0xDC
#define S3C6410_VIDW02ADD1      0x00E0
#define S3C6410_VIDW03ADD1      0x00E8
#define S3C6410_VIDW04ADD1      0x00F0
#define S3C6410_VIDW00ADD2      0x0100
#define S3C6410_VIDW01ADD2      0x0104
#define S3C6410_VIDW02ADD2      0x0108
#define S3C6410_VIDW03ADD2      0x010C
#define S3C6410_VIDW04ADD2      0x0110
#define S3C6410_VIDINTCON0      0x0130
#define S3C6410_VIDINTCON1      0x0134
#define S3C6410_W1KEYCON0       0x0140
#define S3C6410_W1KEYCON1       0x0144
#define S3C6410_W2KEYCON0       0x0148
#define S3C6410_W2KEYCON1       0x014C
#define S3C6410_W3KEYCON0       0x0150
#define S3C6410_W3KEYCON1       0x0154
#define S3C6410_W4KEYCON0       0x0158
#define S3C6410_W4KEYCON1       0x015C
//
#define BPP16_TFT    12
#define BPP24_TFT    13


#define HCLOCK           100000000
#define PCLOCK_50M        50000000


// NAND is OK
#define NAND_CTRL_BASE 0x70200000
#define NFCONF        0x00
#define NFCONT        0x04
#define NFCMMD        0x08
#define NFADDR        0x0c
#define NFDATA        0x10
#define NFSTAT        0x28
#define NFMECC0       0x34

#define tALS  12
#define tWP   12
#define tALH  5

#define TACLS  (tALS - tWP)
#define TWRPH0  tWP
#define TWRPH1  tALH


// UART is OK
#define UART_BASE    0x7F005000
//#define UART_BASE(n)  (UART_BASE + (n) * xxx)
#define ULCON        0x00
#define UCON         0x04
#define UFCON        0x08
#define UMCON        0x0C
#define UTRSTAT      0x10
#define UERSTAT      0x14
#define UFSTAT       0x18
#define UMSTAT       0x1C
#define UTXH         0x20
#define URXH         0x24
#define UBRDIV       0x28
#define UDIVSLOT     0x2C

#define CONFIG_UART_ENABLE_FIFO
#define CURR_UART_BASE   (UART_BASE + CONFIG_UART_INDEX * 0x4000)

// fixme: to support CONFIG_DEFAULT_UART
#define s3c_uart_readb(reg)        readb(VA(UART_BASE + reg))
#define s3c_uart_readl(reg)        readl(VA(UART_BASE + reg))
#define s3c_uart_writeb(reg, val)  writeb(VA(UART_BASE + reg), val)
#define s3c_uart_writel(reg, val)  writel(VA(UART_BASE + reg), val)


// SDRAMC is OK
#define DRAMC_BASE     0x7E001000
#define P1MEMSTAT      0x000
#define P1MEMCCMD      0x004
#define P1DIRECTCMD    0x008
#define P1MEMCFG       0x00C
#define P1REFRESH      0x010
#define P1CASLAT       0x014
#define P1T_DQSS       0x018
#define P1T_MRD        0x01C
#define P1T_RAS        0x020
#define P1T_RC         0x024
#define P1T_RCD        0x028
#define P1T_RFC        0x02C
#define P1T_RP         0x030
#define P1T_RRD        0x034
#define P1T_WR         0x038
#define P1T_WTR        0x03C
#define P1T_XP         0x040
#define P1T_XSR        0x044
#define P1T_ESR        0x048
#define P1MEMCFG2      0x04C
#define P1ID_0_CFG     0x100
#define P1CHIP_0_CFG   0x200
#define P1CHIP_1_CFG   0x204
#define P1USER_STAT    0x300
#define P1USER_CFG     0x304


// Watchdog
#define WATCHDOG_BASE   0x7E004000
#define WTCON                 0x00
#define WTDATA                0x04
#define WTCNT                 0x08


// GPIO
#define GPIO_BASE       0x56000000
//
#define GPF_CON               0x50
//
#define GPG_CON               0x60
#define GPG_DAT               0x64
#define GPG_UP                0x68
//
#define GPH_CON               0x70
#define GPH_DAT               0x74
#define GPH_UP                0x78
//...
#define EXTINT0               0x88
#define EXTINT1               0x8c
#define EXTINT2               0x90
//...
#define EINTMASK              0xa4
#define EINTPEND              0xa8


// RTC
#define RTC_BASE        0x57000040
#define RTCCON                0x00
#define RTCCNT                0x04

#define PWM_BASE        0x51000000
#define TCFG0				  0x00
#define TCFG1				  0x04
#define TCON				  0x08
#define TCNTB(n)			  (0x0c * (n + 1))
#define TCMPB(n)			  (0x10 + 0xc * n)
#define TCNTO(n)			  (0x14 + 0xc * n)


#define S3C64XX_GPBCON   0x56000010
#define S3C64XX_GPBDAT   0x56000014
#define S3C64XX_GPBUP    0x56000018

#define S3C64XX_GPCCON   0x56000020
#define S3C64XX_GPCDAT   0x56000024
#define S3C64XX_GPCUP    0x56000028

#define S3C64XX_GPDCON   0x56000030
#define S3C64XX_GPDDAT   0x56000034
#define S3C64XX_GPDUP    0x56000038

#define S3C64XX_GPECON   0x56000040
#define S3C64XX_GPEDAT   0x56000044
#define S3C64XX_GPEUP    0x56000048

#define S3C64XX_GPFCON   0x56000050
#define S3C64XX_GPFDAT   0x56000054
#define S3C64XX_GPFUP    0x56000058

// SYSCON is OK
#define SYSCON_BASE  0x7E00F000
#define APLL_LOCK    0x000
#define MPLL_LOCK    0x004
#define EPLL_LOCK    0x008
#define APLL_CON     0x00C
#define MPLL_CON     0x010
#define EPLL_CON0    0x014
#define EPLL_CON1    0x018
#define CLK_SRC      0x01C
#define CLK_DIV0     0x020
#define CLK_DIV1     0x024
#define CLK_DIV2     0x028
#define MISC_CON     0x838
#define OTHERS       0x900

#define syscon_write(reg, val)  writel(VA(SYSCON_BASE + reg), val)
#define syscon_read(reg)        readl(VA(SYSCON_BASE + reg))

// SROM controller
#define SROM_BASE    0x70000000
#define SROM_BW      0x00
#define SROM_BC0     0x04
#define SROM_BC1     0x08

#define MMC0_BASE    0x7c200000
#define MMC1_BASE    0x7c300000
#define MMC2_BASE    0x7c400000
#define BLKSIZE      0x04
#define BLKCNT       0x06
#define ARGUMENT     0x08
#define TRANSMOD     0x0c
#define CMDREG       0x0e
#define RESPREG0     0x10
#define RESPREG1     0x14
#define RESPREG2     0x18
#define RESPREG3     0x1c
#define BDAT         0x20
#define PRNSTS       0x24
#define HOSTCTRL     0x28
#define PWRCON      0x29
#define BLKGAP       0x2a
#define WAKUP        0x2b
#define CLKCON       0x2c
#define TIMEOUTCON   0x2e
#define SWRST        0x2f
#define NORINTSTS    0x30
#define ERRINTSTS    0x32
#define NORINTSTSEN  0x34
#define ERRINTSTSE   0x36
#define NORINTSIGEN  0x38
#define ERRINTSIGEN  0x3a
#define ACMD12ERRSTS 0x3c
#define CAPAREG      0x40
#define MAXCURR      0x48
#define CONTRL2      0x80
#define CONTRL3      0x84
#define CONTRL4      0x8c
#define FEAER        0x50
#define FEERR        0x52
#define ADMAERR      0x54
#define ADMASYSADDR  0x58
#define HCVER        0xfe
