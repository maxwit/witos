#include <io.h>
#include <delay.h>
#include <uart/uart.h>

static void omap3_uart_send_byte(__u8 b)
{
	while (readb(VA(UART_BASE + SSR_REG)) & 0x1);

	writeb(VA(UART_BASE + THR_REG), b);
}

static __u8 omap3_uart_recv_byte(void)
{
	while (!(readb(VA(UART_BASE + LSR_REG)) & 0x1));

	return readb(VA(UART_BASE + RHR_REG));
}

__u32 uart_rxbuf_count()
{
	return readb(VA(UART_BASE + LSR_REG)) & 0x1;
}

static int omap3_uart_init(void)
{
	__u8	byte;
	__u16 half;
	__u32 word;

#if 0
	//select source 48M
	word = readl(VA(CM_CLKSEL1_PLL));
	word |= 0x1 << 3;
	writel(VA(CM_CLKSEL1_PLL), word);
#endif

	word = readl(VA(CM_CLKSEL3_PLL));
	word &= ~0x1F;
	word |= 0x9;
	writel(VA(CM_CLKSEL3_PLL), word);

	word = readl(VA(CM_CLKSEL2_PLL));
	word &= ~(0x7FF << 8 | 0x7F);
	word |= 0x1B0 << 8 | 0xC;
	writel(VA(CM_CLKSEL2_PLL), word);

	word = readl(VA(CM_CLKEN_PLL));
	word &= ~(0xF << 20);
	word |= 0x37 << 16;
	writel(VA(CM_CLKEN_PLL), word);

#if (CONFIG_UART_INDEX==0)
	// Config UART1 GPIO
	word = readl(VA(PADCONF_UART1_TX));
	word &= ~(0x7 << 16 | 0x7);
	writel(VA(PADCONF_UART1_TX), word);

	word = readl(VA(PADCONF_UART1_CTS));
	word &= ~(0x7 << 16 | 0x7);
	writel(VA(PADCONF_UART1_CTS), word);

	// Enable uart1's fclk
	word = readl(VA(CM_FCLKEN1_CORE));
	word |= 1 << 13;
	writel(VA(CM_FCLKEN1_CORE), word);

	// Enable uart1's iclk
	word = readl(VA(CM_ICLKEN1_CORE));
	word |= 1 << 13;
	writel(VA(CM_ICLKEN1_CORE), word);
#elif (CONFIG_UART_INDEX==2)
	// GPIO_165 config mode 0.
	// half = readw(VA(PADCONF_UART3_RT));
	half = 1 << 8;
	writew(VA(PADCONF_UART3_RT), half);

	// GPIO_166 config mode 0.
	half = readw(VA(PADCONF_UART3_TX));
	half &= ~0x7;
	writew(VA(PADCONF_UART3_TX), half);

	// enable uart3's fclk
	word = readl(VA(CM_FCLKEN_PER));
	word |= 1 << 11;
	writel(VA(CM_FCLKEN_PER), word);
#endif

	// writel(VA(PADCONF_UART3_RX_IRRX), 1 << 8);
	// writel(VA(PADCONF_UART3_TX_IRTX), 0);
#if 0
	//uart clock init
	//enable uart1 and uart2's fclk
	word = readl(VA(CM_FCLKEN1_CORE));
	word |= 0x3 << 13;
	writel(VA(CM_FCLKEN1_CORE), word);

	#if 1
	//enable auto-idle.
	word = readl(VA(CM_AUTOIDLE_PER));
	word |= 1 << 11;
	writel(VA(CM_AUTOIDLE_PER), word);

	//enable uart3's Iclk
	word = readl(VA(CM_ICLKEN_PER));
	word |= 1 << 11;
	writel(VA(CM_ICLKEN_PER), word);
	#endif

	#if 0
	word = readl(VA(CM_CLKSEL1_PLL));
	word &= ~(1 << 6);
	word &= ~(1 << 3);
	writel(VA(CM_CLKSEL1_PLL), word);
	#endif

	//software reset
	//reset uart1
	byte = readb(VA(UART1_BASE + SYSC_REG));
	byte |= 2;
	writeb(VA(UART1_BASE + SYSC_REG), byte);
	while (!(readb(VA(UART1_BASE + SYSS_REG)) & 0x1));

	//reset uart2
	byte = readb(VA(UART2_BASE + SYSC_REG));
	byte |= 2;
	writeb(VA(UART2_BASE + SYSC_REG), byte);
	while (!(readb(VA(UART2_BASE + SYSS_REG)) & 0x1));
#endif

	// reset uart
	byte = readb(VA(UART_BASE + SYSC_REG));
	byte |= 0x2;
	writeb(VA(UART_BASE + SYSC_REG), byte);
	while (!(readb(VA(UART_BASE + SYSS_REG)) & 0x1));

#if 0
	//switch to mode config B
	writeb(VA(UART_BASE + LCR_REG), 0x00BF);

	//set uart3 uart*16 mode
	writeb(VA(UART_BASE + MDR1_REG), 0);

	// enable uart3's fclk
	word = readl(VA(CM_FCLKEN_PER));
	word |= 1 << 11;
	writel(VA(CM_FCLKEN_PER), word);

	//set FIFO enable and clean the FIFO
	byte = readb(VA(UART_BASE + FCR_REG));
	byte |= 0x7;
	writeb(VA(UART_BASE + FCR_REG), byte);
	//wait for moment?

	writeb(VA(UART_BASE + DLL_REG), 26);

	//set the UART format and return to operation mode
	byte = 0x3;
	writeb(VA(UART_BASE + LCR_REG), byte);
#endif

	half = readw(VA(UART_BASE + MDR1_REG));
	half |= 0x7;
	writew(VA(UART_BASE + MDR1_REG), half);

	writew(VA(UART_BASE + LCR_REG), 0x00BF);

#if 0
	half = readw(VA(UART_BASE + EFR_REG));
	half |= 0x1 << 4;
	writew(VA(UART_BASE + EFR_REG), half);
#endif

	writew(VA(UART_BASE + LCR_REG), 0x0);

	writew(VA(UART_BASE + IER_REG), 0x0);

	writew(VA(UART_BASE + LCR_REG), 0x00BF);

	// enable uart3's fclk
	word = readl(VA(CM_FCLKEN_PER));
	word |= 1 << 11;
	writel(VA(CM_FCLKEN_PER), word);

	half = readw(VA(UART_BASE + DLL_REG));
	half &= ~0xFF;
	half |= 0x1A;
	writew(VA(UART_BASE + DLL_REG), half);

	half = readw(VA(UART_BASE + DLH_REG));
	half &= ~0x3F;
	writew(VA(UART_BASE + DLH_REG), half);

	writew(VA(UART_BASE + LCR_REG), 0x0);

	writew(VA(UART_BASE + IER_REG), 0x0);

	writew(VA(UART_BASE + LCR_REG), 0x3);

	// should enable FIFO before when the baud clock is not running
	// half = readw(VA(UART_BASE + FCR_REG));
	half = 0x7;
	writew(VA(UART_BASE + FCR_REG),half);

	half = readw(VA(UART_BASE + LCR_REG));
	half &= ~(0x3 << 6);
	writew(VA(UART_BASE + LCR_REG), half);

	half = readw(VA(UART_BASE + MDR1_REG));
	half &= ~0x7;
	writew(VA(UART_BASE + MDR1_REG), half);

	udelay(0x1000);
#if 0
	udelay(1000000);
	omap3_uart_send_byte('x');
	udelay(1000000);
	omap3_uart_send_byte('y');
	udelay(1000000);
	omap3_uart_send_byte('z');
	udelay(1000000);

	udelay(10000);
	omap3_uart_send_byte('6');
	udelay(10000);
	omap3_uart_send_byte('5');
	udelay(10000);
	omap3_uart_send_byte('4');
	udelay(10000);
	omap3_uart_send_byte('3');
#endif

	return 0;
}

DECLARE_UART_INIT(omap3_uart_init);
DECLARE_UART_SEND(omap3_uart_send_byte);
DECLARE_UART_RECV(omap3_uart_recv_byte);
