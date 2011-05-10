#include <errno.h>
#include <loader.h>
#include <uart/uart.h>
#include <uart/kermit.h>
#include <flash/part.h>
#ifdef CONFIG_DEBUG
#include <stdio.h>
#endif

#define MARK_START  0x1
#define MARK_EXIT   0x3

#define KERM_TYPE_DATA  'D'
#define KERM_TYPE_SEND  'S'
#define KERM_TYPE_ACK   'Y'
#define KERM_TYPE_NACK  'N'
#define KERM_TYPE_HEAD  'F'
#define KERM_TYPE_BREAK 'B'

#define KERM_KEY_SPACE   0x20
#define KERM_KEY_SHARP   0x23
#define KERM_KEY_TERM    0x0d  /* '\n' */

#define KERM_BUF_LEN   128
#define KERM_ACK_LEN   16

#define ENC_PRINT(c) (c + KERM_KEY_SPACE)
#define DEC_PRINT(c) (c - KERM_KEY_SPACE)

static void send_ack_packet(u32 seq, char type)
{
	u8 buff[KERM_ACK_LEN];
	int index = 0, check_sum = 0;

	buff[index++] = MARK_START;
	buff[index++] = ENC_PRINT(3);
	buff[index++] = seq + KERM_KEY_SPACE;
	buff[index++] = type;
	buff[index]   = '\0';

	index = 1;
	while (buff[index])
	{
		check_sum += buff[index];
		index++;
	}

	buff[index++] = (KERM_KEY_SPACE + (0x3f & (check_sum + (0x03 & (check_sum >> 6))))) & 0xff;
	buff[index++] = KERM_KEY_TERM;
	buff[index] = '\0';

	index = 0;
	while (buff[index])
	{
		uart_send_byte(buff[index]);
		index++;
	}
}

int kermit_load(struct loader_opt *opt)
{
	u8 buff[KERM_BUF_LEN];
	u8 curr_char;
	int index, count, check_sum, len, seq, real_seq = 0;
	int type = KERM_TYPE_BREAK; // fixme
	u8 *curr_addr = (u8 *)opt->load_addr;
	int i;

#ifndef CONFIG_GTH
	if (!opt->load_addr)
	{
		u8 data[KERM_BUF_LEN];
		curr_addr = data;
	}
	else
#endif
	{
		curr_addr = opt->load_addr;
	}
	opt->load_size = 0;

	do
	{
		while (MARK_START != uart_recv_byte());

		for (index = 0; ; index++)
		{
			buff[index] = uart_recv_byte();

			if (KERM_KEY_TERM == buff[index])
				break;
		}

		index = 0;
		count = 0;

		/* length decode */
		len = buff[index++];
		check_sum = len;
		len -= KERM_KEY_SPACE;

		/* sequence decode */
		seq = buff[index++];
		check_sum += seq;
		seq -= KERM_KEY_SPACE;

		if (seq != real_seq)
		{
			send_ack_packet(real_seq, KERM_TYPE_NACK);
			continue;
		}

		real_seq = (real_seq + 1) & 63;

		/* get package type */
		type = buff[index++];
		check_sum += type;

		if (len) // fixme: handle extended length
			len -= 2;

		switch (type)
		{
		case KERM_TYPE_HEAD:
			i = 0;
			while (len > 1)
			{
				curr_char = buff[index++];
				check_sum += curr_char;
				len--;

				if (curr_char == KERM_KEY_SHARP)
				{
					curr_char = buff[index++];
					check_sum += curr_char;
					len--;

					if (0x40 == (curr_char & 0x60))
						curr_char = curr_char & ~0x40;
					else if (0x3f == (curr_char & 0x7f))
						curr_char |= 0x40;
				}

				opt->file_name[i++] = curr_char;
			}
			opt->file_name[i] = '\0';
			break;

		case KERM_TYPE_DATA:
			while (len > 1)
			{
				curr_char = buff[index++];
				check_sum += curr_char;
				len--;

				if (curr_char == KERM_KEY_SHARP)
				{
					curr_char = buff[index++];
					check_sum += curr_char;
					len--;

					if (0x40 == (curr_char & 0x60))
						curr_char = curr_char & ~0x40;
					else if (0x3f == (curr_char & 0x7f))
						curr_char |= 0x40;
				}

				curr_addr[count++] = curr_char;
			}
			break;

		default:
			while (len > 1)
			{
				curr_char = buff[index++];
				check_sum += curr_char;
				len--;
			}
			break;
		}

		/* checksum */
		curr_char = buff[index++];
		if (curr_char != (KERM_KEY_SPACE + (0x3f & (check_sum + (0x03 & (check_sum >> 6))))))
		{
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
			goto Error;

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

Error:
	return -EFAULT;
}

REGISTER_LOADER(k, kermit_load, "Kermit");
