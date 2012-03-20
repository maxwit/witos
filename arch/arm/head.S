/*
 * the entry of g-bios bottom half
 */

	.global _start

_start:
	b     handle_reset
	ldr   pc, [pc, #24]
	ldr   pc, [pc, #24]
	ldr   pc, [pc, #24]
	ldr   pc, [pc, #24]
	.word (_end - _start) @ for AT91SAM926X
	ldr   pc, [pc, #20]
	ldr   pc, [pc, #20]

	.word GBH_MAGIC
	.word handle_undef
	.word handle_swi
	.word handle_iabt
	.word handle_dabt
	.word handle_irq
	.word handle_fiq

handle_reset:
#ifdef CONFIG_NORMAL_SPACE
	ldr   r0, =(_end + CONFIG_HEAP_SIZE + CONFIG_STACK_SIZE)
#else
	ldr   r0, =(_start - CONFIG_HEAP_SIZE)
#endif

	msr   cpsr, #(ARM_MODE_UND | ARM_INT_MASK)
	mov   sp, r0
	sub   r0, r0, #UND_STACK_SIZE

	msr   cpsr, #(ARM_MODE_ABT | ARM_INT_MASK)
	mov   sp, r0
	sub   r0, r0, #ABT_STACK_SIZE

	msr   cpsr, #(ARM_MODE_IRQ | ARM_INT_MASK)
	mov   sp, r0
	sub   r0, r0, #IRQ_STACK_SIZE

	msr   cpsr, #(ARM_MODE_FIQ | ARM_INT_MASK)
	mov   sp, r0
	sub   r0, r0, #FIQ_STACK_SIZE

	msr   cpsr, #(ARM_MODE_SVC | ARM_INT_MASK)
	mov   sp, r0

	ldr   r0, =_start
	adr   r1, _start
	cmp   r0, r1
	beq   skip_move
	ldr   r2, [r1, #GBH_SIZE_OFFSET]
	@ fixme
	bl    memmove
#ifdef CONFIG_DEBUG
	mov   r0, #'M'
	bl	  uart_send_byte
	mov   r0, #'V'
	bl	  uart_send_byte
	mov   r0, #'\n'
	bl	  uart_send_byte
	mov   r0, #'\r'
	bl	  uart_send_byte
#endif
	ldr   pc, =skip_move

skip_move:
	@ fixme for ROM
	mov   r0, #0
	ldr   r1, =_start
	cmp   r1, r0
	beq   bss_init
#ifdef CONFIG_IRQ_SUPPORT
	mov   r2, #(handle_reset - _start)
#ifdef CONFIG_OMAP3
	ldr   r0, =INT_VECTOR_RAM_BASE
	add   r1, r1, #4
	sub   r2, r2, #4
#endif
	bl    memcpy
#endif

bss_init:
	ldr   r0, =bss_begin
	ldr   r1, =bss_end
	mov   r2, #0
0:
	cmp   r0, r1
	strlo r2, [r0], #4
	blo   0b

	mov   fp, #0
	bl    heap_init

	bl    main

	mov   r0, #'n'
	bl	  uart_send_byte
0:
	b     0b

handle_undef:
	stmfd sp!, {r0 - r12, lr}
	bl    undef_handle
	ldmfd sp!, {r0 - r12, pc}^

handle_swi:
	stmfd sp!, {r0 - r12, lr}
	bl    swi_handle
	ldmfd sp!, {r0 - r12, pc}^

handle_iabt:
	sub   lr, lr, #4
	stmfd sp!, {r0 - r12, lr}
	bl    iabt_handle
	ldmfd sp!, {r0 - r12, pc}^

handle_dabt:
	sub   lr, lr, #8
	stmfd sp!, {r0 - r12, lr}
	bl    dabt_handle
	ldmfd sp!, {r0 - r12, pc}^

handle_irq:
	sub   lr, lr, #4
	stmfd sp!, {r0 - r12, lr}
	bl    read_irq_num
	bl    irq_handle
	ldmfd sp!, {r0 - r12, pc}^

handle_fiq:
	sub   lr, lr, #4
	stmfd sp!, {r0 - r7, lr}
	bl    fiq_handle
	ldmfd sp!, {r0 - r7, pc}^
