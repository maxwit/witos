#pragma once

#define WAIT_INFINITE   0
#define WAIT_ASYNC     -1

#define BR115200       115200

#define UART_IOC_RSTFIFO    1
#define UART_IOCG_RXCOUNT   2
#define UART_IOCG_TXCOUNT   3
#define UART_IOCS_TIMEOUT   4

#define CLRSCREEN      "\033[H\033[J" // Esc + '[ '+  '2' + 'J'  ("\033[2J")
#define CHAR_CTRL_C    '\3'
#define CHAR_ESC       '\033'

#define UART_DELAY    (1000000 * 4 / BR115200)

#ifndef __ASSEMBLY__
#include <types.h>

int uart_read(int id, __u8 *buff, int count, int timeout);

void uart_send_byte(__u8 b);

__u8 uart_recv_byte();

__u32 uart_rxbuf_count();

int uart_ioctl(int id,int cmd,void * arg);

int uart_recv_byte_timeout(__u8 *ch, int timeout);

#define DECLARE_UART_INIT(func) \
	 int uart_init(void) __attribute__((alias(#func)))

#define DECLARE_UART_SEND(func) \
	 void uart_send_byte(__u8 b) __attribute__((alias(#func)))

#define DECLARE_UART_RECV(func) \
	 __u8 uart_recv_byte() __attribute__((alias(#func)))
#endif
