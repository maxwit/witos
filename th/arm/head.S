/*
 *
 *  comment here
 *
 */

	.global _start
	.global hang

#ifdef CONFIG_OMAP3
	.word (_end - _start)
	.word _start
#endif
_start:
	b     handle_reset
	ldr   pc, [pc, #24]
	ldr   pc, [pc, #24]
	ldr   pc, [pc, #24]
	ldr   pc, [pc, #24]
	.word (_end - _start)   @ for AT91SAM926X
	ldr   pc, [pc, #20]
	ldr   pc, [pc, #20]

	.word GTH_MAGIC
	.word handle_undef
	.word handle_swi
	.word handle_iabt
	.word handle_dabt
	.word handle_irq
	.word handle_fiq

handle_reset:
	msr  cpsr, #(ARM_MODE_SVC | ARM_INT_MASK)

@ fixme
#if 0
	@ init default sp
#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
	mov  sp, #0x1000
	ldr  r0, [sp, #-4]
	mvn  r1, r0
	str  r1, [sp, #-4]
	ldr  r0, [sp, #-4]
	cmp  r0, r1
	addne sp, sp, #0x40000000
#elif defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9263)
	mov  r0, #0x10000
#endif
#endif

#ifdef CONFIG_S3C6410
	ldr r0, =0x70000013
	mcr p15, 0, r0, c15, c2, 4

	@disable watchdog
	ldr r0, =WATCHDOG_BASE
	ldrh r1, [r0, #WTCON]
	bic r1, #0xff
	strh r1, [r0, #WTCON]
#endif

#ifdef CONFIG_OMAP3
	mov r12, #0x1		@ to invalidate L2
	.word 0xE1600070	@ Call SMI monitor
#endif

	bl   cpu_init

	bl   soc_init

	bl   uart_init

	@ modification:
	@ 1. use adr, instead of ldr=
	@ 2. use loop, instead of puts
	@ 3. remove banner
	adr  r4, build_time
	ldrb r0, [r4], #1
	b    1f
0:
	bl   uart_send_byte
	ldrb r0, [r4], #1
1:
	cmp  r0, #0x0
	bne  0b

	bl   mem_init
	@ 0: skipping memory testing and sp update
	@ othersize: to verify memory R/W and use r0 to update sp
	cmp  r0, #0
	beq  Lm
	@ stack init
	mov  sp, r0
	@ and fp ...

	@ SDRAM testing
	ldr  r2, [sp, #-4]
	mvn  r0, r2
	str  r0, [sp, #-4]
	ldr  r3, [sp, #-4]
	cmp  r0, r3
	beq  Lm
#ifdef CONFIG_DEBUG
	sub  r1, sp, #4
	adr  r0, sdram_fmt
	bl   printf
#endif
	mov  r0, #'M'
	b    hang

Lm:
	@ SDRAM is fully functional now :)
	bl   main
	b    hang

#if 1
handle_undef:
	mov  r0, #'U'
	b    hang

handle_swi:
	mov  r0, #'S'
	b    hang

handle_iabt:
	mov  r0, #'i'
	b    hang

handle_dabt:
	mov  r0, #'d'
	b    hang

handle_irq:
	mov  r0, #'I'
	b    hang

handle_fiq:
	mov  r0, #'F'

#else
handle_undef:
handle_swi:
handle_iabt:
handle_dabt:
handle_irq:
handle_fiq:
	mov  r0, #'E'
#endif

hang:
	bl   uart_send_byte
	sub  pc, pc, #8

build_time:
	.ascii "\n\r\n\r[" __TIME__ "] g-bios\n\r\0"
	.align 2

#ifdef CONFIG_DEBUG
sdram_fmt:
	.asciz "%p: %x, %x\n"
	.align 2
#endif

