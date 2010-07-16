#pragma once

#include <arm/board_id.h>
#include <arm/s3c6410_reg.h>

#define SDRAM_BASE      0x50000000
#define SDRAM_SIZE      0x04000000

#define UART_NUM         5

#define S3C6410_SRAM_BASE 0x0c000000
#define S3C6410_SRAM_SIZE 0x1000

///
#define FIN       12000000

#define A_MDIV    333
#define A_PDIV    3
#define A_SDIV    1

#define M_MDIV    266
#define M_PDIV    3
#define M_SDIV    2

#define AM_PLL_RATE(mdiv, pdiv, sdiv) \
	(FIN / ((pdiv) * (1 << (sdiv))) * (mdiv))

#define APLL_RATE AM_PLL_RATE(A_MDIV, A_PDIV, A_SDIV)
#define MPLL_RATE AM_PLL_RATE(M_MDIV, M_PDIV, M_SDIV)

#define ACLK_RATE    APLL_RATE
#define HCLKx2_RATE  MPLL_RATE
#define HCLK_RATE   (MPLL_RATE / 2)
#define PCLK_RATE   (HCLK_RATE / 2)

// fixme
#define ATAG_BASE          SDRAM_BASE

#define INITRD_BASE_ADDR   (SDRAM_BASE + SDRAM_SIZE / 4)
#define INITRD_MAX_SIZE    (SDRAM_SIZE / 4)

#define LINUX_BASE_ADDR    (INITRD_BASE_ADDR + SDRAM_SIZE / 4)
#define LINUX_MAX_SIZE     (SDRAM_SIZE / 4)
