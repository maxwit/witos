
	.global tputs
tputs:
	stmfd sp!, {lr}
	@ mov   r2,  #0x50000000
	mov   r2,  #CURR_UART_BASE
	b     .L2
.L1:
	@ ldr   r3,  [r2, #0x18]
	ldr   r3,  [r2, #UFSTAT]
	tst   r3,  #(1 << 14)
	bne   .L1
	strb  r1,  [r2, #UTX]
.L2:
	ldrb  r1,  [r0], #1
	cmp   r1,  #0
	bne   .L1
	ldmfd sp!, {pc}
