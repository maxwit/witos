#include <arm/s3c24x0.h>
#include <uart/uart.h>

#define UART_BASE(num) VA((UART0_BASE + (num) * 0x4000))

static int s3c24x0_uart_init(void)
{
	int num;

	writel(VA(GPIO_BASE + GPH_CON), 0x16faaa);
	writel(VA(GPIO_BASE + GPH_UP), 0x7ff);

	for (num = 0; num < UART_NUM; num++) {
		writel(UART_BASE(num) + ULCON, 0x3);
		writel(UART_BASE(num) + UCON, 0x245);

#ifdef CONFIG_UART_ENABLE_FIFO
		writel(UART_BASE(num) + UFCON, 1);
#else
		writel(UART_BASE(num) + UFCON, 0);
#endif

		writel(UART_BASE(num) + UBRDIV, (PCLK_RATE / (BR115200 * 16)) - 1);
	}

	return 0;
}

static __u8 s3c24x0_uart_recv_byte()
{
#ifdef CONFIG_UART_ENABLE_FIFO
	while (!(readl(VA(CURR_UART_BASE + UFSTAT)) & RX_COUNT));
#else
	while (!(readl(VA(CURR_UART_BASE + UTRSTAT)) & 0x1));
#endif

	return readb(VA(CURR_UART_BASE + URX));
}

static void s3c24x0_uart_send_byte(__u8 ch)
{
#ifdef CONFIG_UART_ENABLE_FIFO
	while (readl(VA(CURR_UART_BASE + UFSTAT)) & FIFO_FULL);
#else
	while (!(readl(VA(CURR_UART_BASE + UTRSTAT)) & 0x2));
#endif

	writeb(VA(CURR_UART_BASE + UTX), ch);
}

__u32 uart_rxbuf_count(void)
{
#ifdef CONFIG_UART_ENABLE_FIFO
	return readl(VA(CURR_UART_BASE + UFSTAT)) & RX_COUNT;
#else
	return readl(VA(CURR_UART_BASE + UTRSTAT)) & 0x1;
#endif
}

DECLARE_UART_INIT(s3c24x0_uart_init);
DECLARE_UART_RECV(s3c24x0_uart_recv_byte);
DECLARE_UART_SEND(s3c24x0_uart_send_byte);
