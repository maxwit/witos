#include <errno.h>
#include <loader.h>
#include <uart/uart.h>
#include <uart/kermit.h>
#ifdef CONFIG_DEBUG
#include <stdio.h>
#endif

#define MARK_START  0x1
#define MARK_EXIT   0x3

#define KERM_TYPE_DATA  'D'
#define KERM_TYPE_SEND  'S'
#define KERM_TYPE_ACK   'Y'
#define KERM_TYPE_NACK  'N'
#define KERM_TYPE_BREAK 'B'

#define KERM_KEY_SPACE   0x20
#define KERM_KEY_SHARP   0x23
#define KERM_KEY_TERM    0x0d  /* '\n' */

#define KERM_BUF_LEN   128
#define KERM_ACK_LEN   16

#define ENC_PRINT(c) (c + KERM_KEY_SPACE)
#define DEC_PRINT(c) (c - KERM_KEY_SPACE)

static void send_ack_packet(__u32 seq, char type)
{
	__u8 buff[KERM_ACK_LEN];
	int index = 0, checksum = 0;

	buff[index++] = MARK_START;
	buff[index++] = ENC_PRINT(3);
	buff[index++] = seq + KERM_KEY_SPACE;
	buff[index++] = type;
	buff[index]   = '\0';

	index = 1;
	while (buff[index]) {
		checksum += buff[index];
		index++;
	}

	buff[index++] = (KERM_KEY_SPACE + (0x3f & (checksum + (0x03 & (checksum >> 6))))) & 0xff;
	buff[index++] = KERM_KEY_TERM;
	buff[index] = '\0';

	index = 0;
	while (buff[index]) {
		uart_send_byte(buff[index]);
		index++;
	}
}

int kermit_load(struct loader_opt *opt)
{
	__u8 buff[KERM_BUF_LEN];
	__u8 curr_char;
	int index, count, checksum, len, seq, real_seq = 0;
	int type = KERM_TYPE_BREAK; // fixme
	__u8 *curr_addr = (__u8 *)opt->load_addr;

#ifndef CONFIG_GTH
	if (!opt->load_addr) {
		__u8 data[KERM_BUF_LEN];
		curr_addr = data;
		printf("curr_addr = %p\n",data);
	} else
#endif
	{
		curr_addr = opt->load_addr;
	}
	opt->load_size = 0;

	do {
		while (MARK_START != uart_recv_byte());

		for (index = 0; ; index++) {
			buff[index] = uart_recv_byte();

			if (KERM_KEY_TERM == buff[index])
				break;
		}

		index = 0;
		count = 0;

		/* length decode */
		len = buff[index++];
		checksum = len;
		len -= KERM_KEY_SPACE;

		/* sequence decode */
		seq = buff[index++];
		checksum += seq;
		seq -= KERM_KEY_SPACE;

		if (seq != real_seq) {
#ifdef CONFIG_DEBUG
			// while (1)
				printf("SEQ error: 0x%x != 0x%x\n", seq, real_seq);
#endif
			send_ack_packet(real_seq, KERM_TYPE_NACK);
			continue;
		}

		real_seq = (real_seq + 1) & 63;

		/* get package type */
		type = buff[index++];
		checksum += type;

		if (len) // fixme: handle extended length
			len -= 2;

		while (len > 1) {
			curr_char = buff[index++];
			checksum += curr_char;
			len--;

			if (type != KERM_TYPE_DATA)
				continue;

			if (curr_char == KERM_KEY_SHARP) {
				curr_char = buff[index++];
				checksum += curr_char;
				len--;

				if (0x40 == (curr_char & 0x60))
					curr_char = curr_char & ~0x40;
				else if (0x3f == (curr_char & 0x7f))
					curr_char |= 0x40;
			}

			// *curr_addr++ = curr_char;
			curr_addr[count++] = curr_char;
		}

		/* checksum */
		curr_char = buff[index++];
		if (curr_char != (KERM_KEY_SPACE + (0x3f & (checksum + (0x03 & (checksum >> 6)))))) {
#ifdef CONFIG_DEBUG
			// while (1)
				printf("Checksum error!\n");
#endif
			send_ack_packet(real_seq, KERM_TYPE_NACK);
			continue;
		}

		/* terminator */
		curr_char = buff[index++];
		if (curr_char != KERM_KEY_TERM)
			goto error;

#ifndef CONFIG_GTH
		part_write(opt->part, curr_addr, count);

		if (opt->load_addr)
#endif
		{
			curr_addr += count;
		}

		opt->load_size += count;

		/* send ack package */
		send_ack_packet(seq, KERM_TYPE_ACK);
	} while (type != KERM_TYPE_BREAK);

	return opt->load_size;

error:
	return -EFAULT;
}

REGISTER_LOADER(k, kermit_load, "Kermit");
