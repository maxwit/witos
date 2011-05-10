/*
 *  int kermit_load(void *load_addr);
 */

#include <uart/kermit.h>

#define KERM_BUF_LEN   128
#define KERM_ACK_LEN   0x10

	.global  kermit_load
	.global  gth_kermit_load
	.global  gbh_kermit_load

/*
 * r5 <- checksum
 * r6 <- length
 * r7 <- sequence num
 * r8 <- packet type
 */
kermit_load:
	mov   ip, sp
	stmfd sp!, {fp, ip, lr, pc}
	sub   fp, ip, #4
	sub   sp, sp, #(KERM_BUF_LEN + 0x4)
	str   r0, [fp, #-(KERM_BUF_LEN + 0x10)]

__get_pkt:
	bl    uart_recv_byte
	cmp   r0, #MARK_START
	bne   __get_pkt

	sub   r4, fp, #(0xc + KERM_BUF_LEN)
1:
	bl    uart_recv_byte
	strb  r0, [r4], #1
	cmp   r0, #KERM_KEY_TERM
	bne   1b

	sub   r4, fp, #(0xc + KERM_BUF_LEN)

	/* length decode */
	ldrb  r6, [r4], #1
	mov   r5, r6  /* checksum init */
	sub   r6, r6, #KERM_KEY_SPACE

	/* sequence decode */
	ldrb  r7, [r4], #1
	add   r5, r5, r7
	sub   r7, r7, #KERM_KEY_SPACE

	/* read packet type */
	ldrb  r8, [r4], #1
	add   r5, r5, r8

	/* extended length */
	cmp   r6, #0
	subne r6, r6, #2
	bne   exstract_data

exstract_data:
	cmp   r6, #1
	ble   kerm_recv_done

	ldrb  r0, [r4], #1
	add   r5, r5, r0
	sub   r6, r6, #1

	cmp   r8, #KERM_TYPE_DATA
	bne   exstract_data

	cmp   r0, #KERM_KEY_SHARP
	bne   save_byte
	/* read the char coming after '#' */
	ldrb  r0, [r4], #1
	add   r5, r5, r0
	sub   r6, r6, #1

	/* if ((r0 & 0x60) == 0x40) */
	and   r1, r0, #0x60
	cmp   r1, #0x40
	biceq r0, r0, #0x40
	beq   save_byte
	/* else if ((r0 & 0x7f) == 0x3f) */
	and   r1, r0, #0x7f
	cmp   r1, #0x3f
	orreq r0, r0, #0x40

save_byte:
	ldr   r2, [fp, #-(KERM_BUF_LEN + 0x10)]
	strb  r0, [r2], #1
	str   r2, [fp, #-(KERM_BUF_LEN + 0x10)]

	b	 exstract_data
kerm_recv_done:
	/* checksum */
	ldrb  r0, [r4], #1
	mov   r1, r5, lsr #6
	and   r1, r1, #0x03
	add   r1, r1, r5
	and   r1, r1, #0x3f
	add   r1, r1, #KERM_KEY_SPACE
	cmp   r0, r1
	blne  kerm_error

	/* terminator */
	ldrb  r0, [r4], #1
	cmp   r0, #KERM_KEY_TERM
	blne  kerm_error

	bl    send_ack_packet

	cmp   r8, #KERM_TYPE_BREAK
	bne   __get_pkt

	/* all downloaded! */
	sub   sp, fp, #12
	ldmfd sp, {fp, sp, pc}

send_ack_packet:
	mov   ip, sp
	stmfd sp!, {fp, ip, lr, pc}
	sub   fp, ip, #4
	sub   sp, sp, #KERM_ACK_LEN

	/* ack packet */
	sub   r0,  fp, #0x1c

	mov   r1, #MARK_START
	strb  r1, [r0, #0]

	mov   r1, #ENC_PRINT(3)
	strb  r1, [r0, #1]

	add   r7, r7, #KERM_KEY_SPACE
	strb  r7, [r0, #2]

	mov   r1, #KERM_TYPE_ACK
	strb  r1, [r0, #3]

	/* CHECK */
	mov   r1, #0
	strb  r1, [r0, #4]

	mov   r3, #0
calc_sum:
	ldrb  r2, [r0, #1]!
	cmp   r2, #0
	addne r3, r3, r2
	bne   calc_sum
	/* make the checksum number */
	mov   r1, r3, lsr #6
	and   r1, r1, #0x03
	add   r1, r1, r3
	and   r1, r1, #0x3f

	add   r1, r1, #KERM_KEY_SPACE
	strb  r1, [r0], #1

	mov   r1, #KERM_KEY_TERM
	strb  r1, [r0], #1

	/* end buf */
	mov   r3, #0
	strb  r3, [r0]

	sub   r0,  fp, #0x1c
	bl    printf

	sub   sp, fp, #12
	ldmfd sp, {fp, sp, pc}

kerm_error:
	mov   r0, #'E'
	bl    uart_recv_byte
	b	  kerm_error
