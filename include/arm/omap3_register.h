#pragma once

#define PRM_CLKSEL			0x48306D40
#define PRM_CLKSRC_CTRL		0x48307270

#define CM_CLK_BASE			0x48004000
#define CM_CLK_REG(x)		(CM_CLK_BASE + (x))
#define CM_CLKEN_PLL_MPU	CM_CLK_REG(0x904)
#define CM_CLKSEL1_PLL_MPU	CM_CLK_REG(0x940)
#define CM_CLKSEL2_PLL_MPU	CM_CLK_REG(0x944)
#define CM_CLKSEL_CORE		CM_CLK_REG(0xA40)
#define CM_FCLKEN1_CORE		CM_CLK_REG(0xA00)
#define CM_FCLKEN_WKUP      CM_CLK_REG(0xC00)
#define CM_ICLKEN_WKUP      CM_CLK_REG(0xC10)
#define CM_IDLEST_WKUP      0x48004C20
#define CM_CLKSEL_WKUP      CM_CLK_REG(0xC40)
#define CM_CLKEN_PLL		CM_CLK_REG(0xD00)
#define CM_CLKSEL1_PLL		CM_CLK_REG(0xD40)
#define CM_CLKSEL2_PLL		CM_CLK_REG(0xD44)
#define CM_CLKSEL3_PLL      CM_CLK_REG(0xD48)
#define CM_ICLKEN1_CORE		CM_CLK_REG(0xA10)

// fixme!
#define PADCONF_UART1_TX   0x4800217C
#define PADCONF_UART1_CTS  0x48002180

#define PADCONF_UART3_RT    0x4800219E
#define PADCONF_UART3_TX    0x480021A0

#define PADCONF_GPMC_NBE1   0x480020C8
#define PADCONF_GPMC_NWE    0x480020C4
#define PADCONF_GPMC_NOE    0x480020C2
#define PADCONF_GPMC_NADV_ALE 0X480020C0

#define CM_FCLKEN_PER       0x48005000
#define CM_ICLKEN_PER       0x48005010
#define CM_CLKSEL_PER       0x48005040
#define CM_AUTOIDLE_PER		0x48005030
#define CM_FCLKEN_PER		0x48005000
#define CM_CLKSEL_PER       0x48005040

#define PLL_STOP        1
#define PLL_LOCK        7
#define PLL_LOW_POWER_BYPASS   5

// fixme!
#define UART1_BASE	0x4806A000
#define UART2_BASE	0x4806C000
#define UART3_BASE	0x49020000

#if (CONFIG_UART_INDEX==0)
#define UART_BASE  UART1_BASE
#elif (CONFIG_UART_INDEX==1)
#define UART_BASE  UART2_BASE
#elif (CONFIG_UART_INDEX==2)
#define UART_BASE  UART3_BASE
#endif

#define DLL_REG	 0x000
#define RHR_REG  0x000
#define THR_REG  0x000
#define IER_REG  0x004
#define FCR_REG  0x008
#define EFR_REG  0x008
#define MCR_REG  0x010
#define LSR_REG  0x014
#define SSR_REG  0x044
#define LCR_REG	 0x00C
#define SYSC_REG 0x054
#define SYSS_REG 0x058
#define MDR1_REG 0x020
#define DLH_REG 0x004

// GPIO
#define GPIO1_BASE		0x48310000
#define GPIO_BASE(i)	(0x49050000 + 0x2000 * (i - 2)) // i = 2 ~ 6
#define GPIO_OE			0x34
#define LEVELDETECT0     0x40
#define LEVELDETECT1     0x44
#define GPIO_IRQ_STATUS1(gpio_index) \
	(((gpio_index) == 0) ? 0x48310018 : 0x49050000 + ((gpio_index) - 1) * 0x2000 + 0x18)

#define GPIO_IRQ_ENABLE1(gpio_index) \
	(((gpio_index) == 0) ? 0x4831001c : 0x49050000 + ((gpio_index) - 1) * 0x2000 + 0x1c)

#define GPIO_IRQ_CLEENABLE1(gpio_index) \
	(((gpio_index) == 0) < 2) ? 0x48310060 : 0x49050000 + ((gpio_index) - 1) * 0x2000 + 0x60)

#define GPIO_IRQ_SETENABLE1(gpio_index) \
	(((gpio_index) == 0) ? 0x48310064 : 0x49050000 + ((gpio_index) - 1) * 0x2000 + 0x64)

#define GPMC_BASE			0x6E000000
//#define	GPMC_REG_ADDR(reg)			(reg + GPMC_BASE)
#define GPMC_IRQ_STATUS		 0x018
#define GPMC_SYSSTATUS			 0x014
#define GPMC_CONFIG				 0x050
#define GPMC_STATUS				 0x054
#define GPMC_CONFIG1_0			 0x060
#define GPMC_CONFIG7_0			 0x078
#define GPMC_NAND_COMMAND_0		 0x07C
#define GPMC_NAND_ADDRESS_0		 0x080
#define GPMC_NAND_DATA_0		 0x084

#define GPMC_CONFIG1(i) (0x6e000060 + 0x30 * (i))
#define GPMC_CONFIG7(i) (0x6e000078 + 0x30 * (i))

#define SYSCONFIG 0x010
#define IRQENABLE 0x01c
#define TIMEOUT   0x040
#define GPMC_CONFIG_CS0 0x60
#define GPMC_CONFIG_1 0x000
#define GPMC_CONFIG_2 0x004
#define GPMC_CONFIG_3 0x008
#define GPMC_CONFIG_4 0x00c
#define GPMC_CONFIG_5 0x010
#define GPMC_CONFIG_6 0x014
#define GPMC_CONFIG_7 0x018

#define GPMC_CS_BASE(x) ((128 * (x)) << 20)

// INTC
#define INTC_PINS	        96
#define MAX_IRQ_NUM	        (INTC_PINS + 32 * 6)
#define GPIO_IRQ(n)		    (INTC_PINS + n)

#define INTCPS_BASE	0x48200000
#define INTCPS_SYSCONFIG	0x010
#define INTCPS_SIR_IRQ		0x040
#define INTCPS_CONTROL		0x048
#define INTCPS_IDLE			0x050
#define INTCPS_MIRN(n)		(0x084 + 0x20 * (n))
#define INTCPS_MIRN_CLEN(n)	(0x088 + 0x20 * (n))
#define INTCPS_MIRN_SETN(n)	(0x08c + 0x20 * (n))
#define INTCPS_ISR_CLEN(n)	(0x094 + 0x20 * (n))
#define INTCPS_ILRM(n)		(0x100 + 0x04 * (n))

#define SDRC_BASE 0x6d000000
#define SDRC_SYSCONFIG	0x010
#define SDRC_SYSSTATUS	0x014
#define SDRC_SHARING	0x044
#define SDRC_MCFG_0		0x080
#define SDRC_MR_0		0x084
#define SDRC_ACTIM_CTRLA_0	0x09c
#define SDRC_ACTIM_CTRLB_0		0x0a0
#define SDRC_RFR_CTRL			0x0a4
#define SDRC_POWER				0x070
#define SDRC_MANUAL_0			0x0a8
#define SDRC_DLLA_CTRL			0x060

//MMC
#define MMCHS1_BASE 0x4809c000
#define MMCHS2_BASE 0x480b4000
#define MMCHS3_BASE 0x480ad000

#define MMCHS_SYSCONFIG 0x10
#define MMCHS_SYSSTATUS 0x14
#define MMCHS_CSRE      0x24
#define MMCHS_SYSTEST   0x28
#define MMCHS_CON       0x2c
#define MMCHS_PWCNT     0x30
#define MMCHS_BLK       0x104
#define MMCHS_ARG       0x108
#define MMCHS_CMD       0x10c
#define MMCHS_RSP10     0x110
#define MMCHS_RSP32     0x114
#define MMCHS_RSP54     0x118
#define MMCHS_RSP76     0x11c
#define MMCHS_DATA      0x120
#define MMCHS_PSTATE    0x124
#define MMCHS_HCTL      0x128
#define MMCHS_SYSCTL    0x12c
#define MMCHS_STAT      0x130
#define MMCHS_IE        0x134
#define MMCHS_ISE       0x138
#define MMCHS_AC12      0x13c
#define MMCHS_CAPA      0x140
#define MMCHS_CUR_CAPA  0x148
#define MMCHS_REV       0x1fc

// WDT
#define WDTTIMER1 0x4830c000
#define WDTTIMER2 0x48314000
#define WDTTIMER3 0x49030000
#define WWPS      0x034
#define WSPR      0x048
#define WIER      0x01c


#define DISPC_BASE          0x48050400

/* DISPC common */
#define DISPC_REVISION			0x0000
#define DISPC_SYSCONFIG			0x0010
#define DISPC_SYSSTATUS			0x0014
#define DISPC_IRQSTATUS			0x0018
#define DISPC_IRQENABLE			0x001C
#define DISPC_CONTROL			0x0040
#define DISPC_CONFIG			0x0044
#define DISPC_CAPABLE			0x0048
#define DISPC_DEFAULT_COLOR0	0x004C
#define DISPC_DEFAULT_COLOR1	0x0050
#define DISPC_TRANS_COLOR0		0x0054
#define DISPC_TRANS_COLOR1		0x0058
#define DISPC_LINE_STATUS		0x005C
#define DISPC_LINE_NUMBER		0x0060
#define DISPC_TIMING_H			0x0064
#define DISPC_TIMING_V			0x0068
#define DISPC_POL_FREQ			0x006C
#define DISPC_DIVISOR			0x0070
#define DISPC_SIZE_DIG			0x0078
#define DISPC_SIZE_LCD			0x007C

#define DISPC_DATA_CYCLE1		0x01D4
#define DISPC_DATA_CYCLE2		0x01D8
#define DISPC_DATA_CYCLE3		0x01DC

/* DISPC GFX plane */
#define DISPC_GFX_BA0			0x0080
#define DISPC_GFX_BA1			0x0084
#define DISPC_GFX_POSITION		0x0088
#define DISPC_GFX_SIZE			0x008C
#define DISPC_GFX_ATTRIBUTES		0x00A0
#define DISPC_GFX_FIFO_THRESHOLD	0x00A4
#define DISPC_GFX_FIFO_SIZE_STATUS	0x00A8
#define DISPC_GFX_ROW_INC		0x00AC
#define DISPC_GFX_PIXEL_INC		0x00B0
#define DISPC_GFX_WINDOW_SKIP	0x00B4
#define DISPC_GFX_TABLE_BA		0x00B8

/* DISPC Video plane 1/2 */
#define DISPC_VID1_BASE			0x00BC
#define DISPC_VID2_BASE			0x014C

#define DISPC_VID0_BA0          0x0bc
#define DISPC_VID0_POSITION		0x0c4
#define DISPC_VID0_SIZE         0x0c8
#define DISPC_VID0_ATTRIBUTES   0x0cc

/* Offsets into DISPC_VID1/2_BASE */
#define DISPC_VID_BA0			0x0000
#define DISPC_VID_BA1			0x0004
#define DISPC_VID_POSITION		0x0008
#define DISPC_VID_SIZE			0x000C
#define DISPC_VID_ATTRIBUTES		0x0010
#define DISPC_VID_FIFO_THRESHOLD	0x0014
#define DISPC_VID_FIFO_SIZE_STATUS	0x0018
#define DISPC_VID_ROW_INC		0x001C
#define DISPC_VID_PIXEL_INC		0x0020
#define DISPC_VID_FIR			0x0024
#define DISPC_VID_PICTURE_SIZE	0x0028
#define DISPC_VID_ACCU0			0x002C
#define DISPC_VID_ACCU1			0x0030

// reset
#define PRM_BASE		0x48307200
#define PRM_RSTCTRL		0x50
