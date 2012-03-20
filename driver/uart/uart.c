#include <errno.h>
#include <delay.h>
#include <uart/uart.h>

int uart_recv_byte_timeout(__u8 *ch, int timeout)
{
	int t = 0;

	while (t < timeout) {
		if (uart_rxbuf_count() > 0) {
			*ch = uart_recv_byte();
			return 0;
		}

		udelay(UART_DELAY);
		t += UART_DELAY;
	}

	return -ETIMEDOUT;
}

int uart_read(int id, __u8 *buff, int count, int timeout)
{
	int size = 0;

	if (WAIT_INFINITE == timeout) {
	}

	if (WAIT_ASYNC == timeout) {
		for (size = 0; size < count; size++) {
			if (uart_rxbuf_count() == 0)
				break;

			buff[size] = uart_recv_byte();
			size++;
		}

		return size;
	}

	//

	return size;
}

void uart_write(int id, const __u8 *buff, int count, int timeout)
{
	while (count > 0) {
		buff++;
		count--;
	}
}

int uart_ioctl(int id, int cmd, void *arg)
{
	switch (cmd) {
	case UART_IOCG_RXCOUNT:
		break;

	case UART_IOC_RSTFIFO:
		break;

	default:
		return -ENOTSUPP;
	}

	return 0;
}

