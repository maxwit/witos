#include <stdio.h>

int main()
{
#if 0
	asm volatile("mov r0, #0x56000000\n\t"
		"ldr r1, [r0, #0x44]\n\t"
		"bic r1, #(1 << 13)\n\t"
		"str r1, [r0, #0x44]\n\t"
		);
#endif
	tputs("Hello, MaxWit!\n\r");

	return 0;
}
