
	.global __udelay
__udelay:
	subs r0, r0, #1
	bhi __udelay
	mov pc, lr
