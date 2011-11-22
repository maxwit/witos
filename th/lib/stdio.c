#include <uart/uart.h>

#define BUF_LEN 10

void putchar(char ch)
{
	uart_send_byte(ch);
	if ('\n' == ch)
		uart_send_byte('\r');
	else if ('\r' == ch)
		uart_send_byte('\n');
}

static const char *int_to_hex_str(__u32 val, char str[])
{
	char *p = str + BUF_LEN;

	do {
		p--;

		*p = val & 0xf;

		if (*p <= 9)
			*p += '0';
		else if (*p <= 0xf)
			*p += 'A' - 10;
		else
			*p = '?';

		val >>= 4;
	} while (val);

	if ((p - str) & 0x1)
		*--p = '0';

	return p;
}

#if ARM_ARCH >= 6
static const char *int_to_oct_str(__u32 val, char str[])
{
	char *p = str + BUF_LEN;

	while (val) {
		p--;

		*p = val % 10;
		if (*p <= 9)
			*p += '0';
		else
			*p = '?';

		val /= 10;
	}

	return p;
}
#endif

// fixme
int printf(const char *fmt, ...)
{
	char buf[BUF_LEN];
	const char *p, *q;
	const __u32 *arg = (const __u32 *)&fmt + 1;

	for (p = fmt; *p != '\0'; p++) {
		if (*p == '%') {
			p++;

			switch (*p) {
			case 'c':
				uart_send_byte((char)*arg);
				break;

			case 'p':
				uart_send_byte('0');
				uart_send_byte('x');
			case 'x':
				q = int_to_hex_str(*arg, buf);
				while (q < buf + BUF_LEN) {
					uart_send_byte(*q);
					q++;
				}
				break;

#if ARM_ARCH >= 6
			case 'd':
				q = int_to_oct_str(*arg, buf);
				while (q < buf + BUF_LEN) {
					uart_send_byte(*q);
					q++;
				}
				break;
#endif

			case 's':
				q = (const char *)*arg;
				while (*q) {
					uart_send_byte(*q);
					if ('\n' == *q)
						uart_send_byte('\r');

					q++;
				}
				break;

			default:
				// uart_send_byte('W');
				return -1;
			}

			arg++;
		} else {
			uart_send_byte(*p);
			if ('\n' == *p)
				uart_send_byte('\r');
		}
	}

	return p - fmt;
}
