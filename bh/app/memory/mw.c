#include <stdio.h>

static void mw_usage(void)
{
	printf("Usage: mw <ADDR> <VAL>\n"
		"Write the VAL to ADDR of memory.\n");
}

int main(int argc, char *argv[])
{
	u32 addr, val;

	mw_usage();
#if 0
	if (argc != 3)
	{
		printf("usage: %s <address> <val>\n", argv[0]);
		return -EINVAL;
	}

	string2value(argv[1], (u32 *)&addr);
	string2value(argv[2], (u32 *)&val);
#endif

	addr = 0;
	writel(VA(addr), 0x0);
	val = readl(VA(addr));
	printf("%x @ %p\n", val, addr);

	writel(VA(addr), ~0x0);
	val = readl(VA(addr));
	printf("%x @ %p\n", val, addr);

	return 0;
}

