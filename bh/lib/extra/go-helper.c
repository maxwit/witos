#include <stdio.h>

static void go_helper()
{
	printf("The jump address is NOT set!\n");
}

static void *g_jump_addr = go_helper;

void *go_get_addr()
{
	return g_jump_addr;
}

void go_set_addr(void *addr)
{
	g_jump_addr = addr;
}
