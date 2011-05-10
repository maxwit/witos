/*
 *  comment here
 */

	.global  mem_init

mem_init:
	stmfd sp!, {lr}
#ifdef CONFIG_AT91SAM9261
	ldr   r0, =AT91SAM926X_PA_MATRIX
	ldr   r1, =0x13a
	str   r1, [r0, #MATRIX_EBICSA]

	mov   r0, #PIOC
	ldr   r1, =0xffff0000
	bl 	  at91_gpio_conf_periA
#elif defined(CONFIG_AT91SAM9263)
	ldr  r0, =AT91SAM926X_PA_MATRIX
	ldr  r1, [r0, #MATRIX_EBI0CSA]
	orr  r1, r1, #(0x1 << 16)
	orr  r1, r1, #(0x1 << 1)
	str  r1, [r0, #MATRIX_EBI0CSA]

	mov   r0, #PIOD
	ldr   r1, =0xffff0000
	bl	  at91_gpio_conf_periA
#endif

	mov  r0, #SDRAM_BASE
	ldr  r1, =AT91SAM926X_PA_SDRAMC
	@ 1
	ldr  r2, =(SDR_tXSR(8) | SDR_tRAS(5) | SDR_tRCD(2) | SDR_tRP(2) | SDR_tRC(7) | SDR_tWR(2) | SDR_CAS(3) | 0x19)
	str  r2, [r1, #SDRAMC_CR]
	@ 3
	mov  r2, #0
	str  r2, [r1, #SDRAMC_MDR]
	@ 4
	ldr  r2, =(MAINCK_RATE / 1000000 * 200)
0:
	subs r2, r2, #1
	bne  0b
	@ 5
	mov  r2, #2
	str  r2, [r1, #SDRAMC_MR]
	str  r2, [r0]
	@ 6
	mov  r2, #4
	str  r2, [r1, #SDRAMC_MR]
0:
	str  r2, [r0]
	add  r2, r2, #1
	cmp  r2, #12
	bls  0b
	@ 7
	mov  r2, #3
	str  r2, [r1, #SDRAMC_MR]
	str  r2, [r0]
	@ 9
	mov  r2, #0
	str  r2, [r1, #SDRAMC_MR]
	str  r2, [r0]
	@ 10
	ldr  r2, =(MCK_RATE / 1000 * 781 / 100 / 1000 + 1)    @ MASTER_CLOCK / 1000000 * 7.81
	str  r2, [r1, #SDRAMC_TR]

	add  r0, r0, #SDRAM_SIZE
	ldmfd sp!, {pc}

