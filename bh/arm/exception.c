#include <stdio.h>

void undef_handle()
{
	printf("Undefined Instruction\n");
	while (1);
}

void swi_handle()
{
	printf("SWI\n");
	while (1);
}

void iabt_handle()
{
	printf("Instruction Abort\n");
	while (1);
}

void dabt_handle()
{
	printf("Data Abort\n");
	while (1);
}

void fiq_handle()
{
	printf("FIQ\n");
	while (1);
}
