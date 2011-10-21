/*
 *  comment here
 */

#include <uart/uart.h>

#define at91_uart_readb(reg)        readb(VA(AT91SAM926X_PA_DBGU + reg))
#define at91_uart_readl(reg)        readl(VA(AT91SAM926X_PA_DBGU + reg))
#define at91_uart_writeb(reg, val)  writeb(VA(AT91SAM926X_PA_DBGU + reg), val)
#define at91_uart_writel(reg, val)  writel(VA(AT91SAM926X_PA_DBGU + reg), val)

static int at91_uart_init(void)
{
#ifdef CONFIG_AT91SAM9261
	at91_gpio_conf_periA(PIOA, 3 << 9, 0);
#elif defined(CONFIG_AT91SAM9263)
	at91_gpio_conf_periA(PIOC, 3 << 30, 0);
#else
#error
#endif

	at91_uart_writel(US_IDR, 0xFFFFFFFF);
	at91_uart_writel(US_CR, 0x10C);
	at91_uart_writel(US_CR, 0x50);
	at91_uart_writel(US_BRGR, MCK_RATE / (BR115200 * 16));
	at91_uart_writel(US_MR, 0x800);
	at91_uart_writel(US_PTCR, 0x101);

	return 0;
}

static __u8 at91_uart_recv_byte()
{
	while ((at91_uart_readl(US_CSR) & 0x1) == 0);
	return at91_uart_readb(US_RHR);
}

static void at91_uart_send_byte(__u8 b)
{
	while ((at91_uart_readl(US_CSR) & 0x2) == 0);
	at91_uart_writeb(US_THR, b);
}

__u32 uart_rxbuf_count(void)
{
	return at91_uart_readl(US_CSR) & 0x1;
}

DECLARE_UART_INIT(at91_uart_init);
DECLARE_UART_RECV(at91_uart_recv_byte);
DECLARE_UART_SEND(at91_uart_send_byte);

