#include <stdio.h>
#include <loader.h>
#include <uart/uart.h>
#include <uart/ymodem.h>

#define SOH    0x01
#define STX    0x02
#define EOT    0x04
#define ACK    0x06
#define NAK    0x15
#define CAN    0x18

#define MODEM_TIMEOUT  (UART_DELAY * 8)

// fixme: reset fifo
static void uart_clear_buff()
{
	__u8  tmp;
	int ret;

	while (1) {
		ret = uart_recv_byte_timeout(&tmp, MODEM_TIMEOUT);

		if (ret < 0)
			break;
	}
}

// TODO:  support MS hyper-term
static inline void modem_end_rx()
{
	uart_send_byte(ACK);
	// uart_send_byte('C');
	// uart_send_byte(ACK);

	uart_send_byte(CAN);
	uart_send_byte(CAN);
	uart_send_byte(CAN);
	uart_send_byte(CAN);
	uart_send_byte(CAN);
	uart_send_byte(CAN);
}

int ymodem_load(struct loader_opt *opt)
{
	int ret;
	int size = 0, count;
	int blk = 0;
	__u8 stx, blk_num[2], crc[2];
	__u8 *curr_addr;

#ifndef CONFIG_GTH
	if (!opt->load_addr) {
		__u8 data[1024];

		curr_addr = data;
	} else
#endif
	{
		curr_addr = opt->load_addr;
	}

	opt->load_size = 0;

	// uart_clear_buff();

	//
	while (1) {
		uart_send_byte('C');

		ret = uart_recv_byte_timeout(&stx, 2000000);

		if (ret < 0)
			continue;

		if (SOH == stx) {
			// printf("SOH (%d)\n", stx);
			size = 128;
			break;
		} else if (STX == stx) {
			// printf("STX (%d)\n", stx);
			size = 1024;
			break;
		}
	}

	blk_num[0] = uart_recv_byte();
	blk_num[1] = uart_recv_byte();

#ifndef CONFIG_GTH
	for (count = 0; count < size; count++) {
		ret = uart_recv_byte_timeout((__u8 *)opt->file_name + count, MODEM_TIMEOUT);

		if (ret == 0) {
			if ('\0' == opt->file_name[count])
				break;
		}
#ifdef CONFIG_DEBUG
		else {
			printf("line %d: timeout!\n", __LINE__);
		}
#endif
	}

#ifdef CONFIG_DEBUG
	printf("loading \'%s\'\n", opt->file_name);
#endif
#endif

	uart_clear_buff();
	uart_send_byte(ACK);

	// receiving data
	uart_send_byte('C');

	blk++;
	while (1) {
		while (1) {
			ret = uart_recv_byte_timeout(&stx, MODEM_TIMEOUT);

			if (ret < 0)
				continue;

			if (SOH == stx) {
				// printf("SOH (%d)\n", stx);
				size = 128;
				break;
			} else if (STX == stx) {
				// printf("STX (%d)\n", stx);
				size = 1024;
				break;
			} else if (EOT == stx) {
				// printf("Done!\n");
				goto L1;
			}
		}

		blk_num[0] = uart_recv_byte();
		blk_num[1] = uart_recv_byte();
		if ((blk_num[0] ^ blk_num[1]) != (__u8)0xFF ||
			blk_num[0] != (__u8)(blk & 0xFF)) {
			uart_clear_buff();
			uart_send_byte(NAK);
#ifdef CONFIG_DEBUG
			printf("block num (%d) error!\n", blk);
#endif
			continue;
		}

		for (count = 0; count < size; count++) {
			ret = uart_recv_byte_timeout(curr_addr + count, MODEM_TIMEOUT);

			if (ret < 0) {
				printf("line %d: timeout.\n");
				break;
			}
		}

		ret = uart_recv_byte_timeout(&crc[0], MODEM_TIMEOUT);
		ret = uart_recv_byte_timeout(&crc[1], MODEM_TIMEOUT);

#ifndef CONFIG_GTH
		part_write(opt->part, curr_addr, count);

		if (opt->load_addr)
#endif
		{
			curr_addr += count;
		}

		uart_send_byte(ACK);

		opt->load_size += count;
		blk++;
	}

L1:
	modem_end_rx();

	return opt->load_size;
}

REGISTER_LOADER(y, ymodem_load, "Y-modem");
