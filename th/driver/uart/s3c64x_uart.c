#include <uart/uart.h>
#include <arm/s3c6410.h>

static int s3c6410_uart_init(void)
{
	__u32 val;

	writel(VA(0x7F008000), 0x22222222);

	val = syscon_read(CLK_DIV2);
	val &= ~(0xf << 16);
	val |= 1 << 16;
	syscon_write(CLK_DIV2, val);

	s3c_uart_writel(ULCON, 0x3);
	s3c_uart_writel(UCON, 0xC05); // or 0x5
#ifdef CONFIG_UART_ENABLE_FIFO
	s3c_uart_writel(UFCON, 0x67);
	while (s3c_uart_readl(UFCON) & 0x6);
#else
	s3c_uart_writel(UFCON, 0x0);
#endif
	s3c_uart_writel(UBRDIV, HCLK_RATE / (BR115200 * 16) - 1);
	s3c_uart_writel(UDIVSLOT, 0x00);

	udelay(100);

	return 0;
}

__u32 uart_rxbuf_count(void)
{
#ifdef CONFIG_UART_ENABLE_FIFO
	return s3c_uart_readl(UFSTAT) & 0x3F;
#else
#error
	return s3c_uart_readl(UTRSTAT) & 0x1;
#endif
}

static __u8 s3c6410_uart_recv_byte()
{
#ifdef CONFIG_UART_ENABLE_FIFO
	while ((s3c_uart_readl(UFSTAT) & 0x3F) == 0);
#else
	while (!(s3c_uart_readl(UTRSTAT) & 0x1));
#endif

	return s3c_uart_readb(URXH);
}

static void s3c6410_uart_send_byte(__u8 b)
{
#ifdef CONFIG_UART_ENABLE_FIFO
	while (s3c_uart_readl(UFSTAT) & (1 << 14));
#else
	while (!(s3c_uart_readl(UTRSTAT) & 0x2));
#endif

	s3c_uart_writeb(UTXH, b);
}

DECLARE_UART_INIT(s3c6410_uart_init);
DECLARE_UART_RECV(s3c6410_uart_recv_byte);
DECLARE_UART_SEND(s3c6410_uart_send_byte);
