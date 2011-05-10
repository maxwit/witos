/*
 *  comment here
 */

	.global  soc_init
	.global  udelay

soc_init:
	@ init sp
	@ fixme: to read SRAM dynamically
	mov   r0,   #0
	msr   cpsr, #(ARM_MODE_UND | ARM_INT_MASK)
	add   sp,   r0, #AT91SAM926X_SRAM_SIZE

	msr   cpsr, #(ARM_MODE_ABT | ARM_INT_MASK)
	add   sp,   r0, #AT91SAM926X_SRAM_SIZE

	msr   cpsr, #(ARM_MODE_IRQ | ARM_INT_MASK)
	add   sp,   r0, #AT91SAM926X_SRAM_SIZE

	msr   cpsr, #(ARM_MODE_FIQ | ARM_INT_MASK)
	add   sp,   r0, #AT91SAM926X_SRAM_SIZE

	msr   cpsr, #(ARM_MODE_SVC | ARM_INT_MASK)
	add   sp,   r0, #AT91SAM926X_SRAM_SIZE

	@ disable WDT
	ldr  r0, =AT91SAM926X_PA_WDTC
	mov  r1, #(0x1 << 15)
	str  r1, [r0, #WDTC_WDMR]

	@ init GPIO
	ldr  r2, =AT91SAM926X_PA_PIOA
	mov  r3, #0x200
	mla  r2, r3, r0, r2
	str  r1, [r2, #PIO_PER]
	str  r1, [r2, #PIO_OER]
	str  r1, [r2, #PIO_CODR]

	@ init clock:
	ldr  r0,  =AT91SAM926X_PA_PMC

	@ MOR
	mov  r1, #0x1
	str  r1, [r0, #PMC_MOR]
0:
	ldr  r1, [r0, #PMC_SR]
	tst  r1, #0x1
	beq  0b

	@ PLLA
	ldr  r1, =(0x20000000 | (MULA << 16) | (0xBF << 8) | DIVA);
	str  r1, [r0, #PMC_PLLAR]
0:
	ldr  r1, [r0, #PMC_SR]
	tst  r1, #0x2
	beq  0b

	@ MCKR
	ldr  r1, =0x102
	str  r1, [r0, #PMC_MCKR]
0:
	ldr  r1, [r0, #PMC_SR]
	tst  r1, #0x8
	beq  0b

	bx   lr

#ifdef CONFIG_GTH
udelay:
	ldr r1, =(ACLK_RATE >> 20) >> 5
	mul r1, r0, r1
0:
	subs r1, r1, #1
	bgt  0b

	bx  lr
#endif

