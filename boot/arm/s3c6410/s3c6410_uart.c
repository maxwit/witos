#include <g-bios.h>
#include <uart/uart.h>
#include <arm/s3c6410.h>


static int s3c6410_uart_init(void)
{
	u32 val;
	volatile int i;

	writel(VA(0x7F008000), 0x22222222);

	val = syscon_read(CLK_DIV2);
	val &= ~(0xf << 16);
	val |= 1 << 16;
	syscon_write(CLK_DIV2, val);

	s3c_uart_writel(ULCON, 0x3);
	s3c_uart_writel(UCON, 0xC05); // 0x5
	s3c_uart_writel(UFCON, 0x67);
	s3c_uart_writel(UBRDIV, HCLK_RATE / (BR115200 * 16) - 1);
	s3c_uart_writel(UDIVSLOT, 0x00);

	for (i = 0; i < 0x1000; i++);

	return 0;
}


u32 uart_rxbuf_count(void)
{
#ifdef CONFIG_UART_ENABLE_FIFO
	return s3c_uart_readl(UFSTAT) & 0x3F;
#else
#error
	return s3c_uart_readl(UTRSTAT) & 0x1;
#endif
}


static u8 s3c6410_uart_recv_byte()
{
#ifdef CONFIG_UART_ENABLE_FIFO
	while ((s3c_uart_readl(UFSTAT) & 0x3F) == 0);
#else
#error
	while (!(s3c_uart_readl(UTRSTAT) & 0x1));
#endif

	return s3c_uart_readb(URXH);
}


static void s3c6410_uart_send_byte(u8 b)
{
#ifdef CONFIG_UART_ENABLE_FIFO
	while (s3c_uart_readl(UFSTAT) & (1 << 14));
#else
#error
	while (!(3c_uart_readl(UTRSTAT) & 0x2));
#endif

	s3c_uart_writeb(UTXH, b);
}


DECLARE_UART_INIT(s3c6410_uart_init);
DECLARE_UART_RECV(s3c6410_uart_recv_byte);
DECLARE_UART_SEND(s3c6410_uart_send_byte);

