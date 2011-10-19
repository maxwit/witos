#include <getopt.h>

#define MD_USAGE          "Usage md [-a <ram address>] [-n <number>] [-h]"

static void usage(const char *cmd)
{
#if 0

Usage: mem <command> [args]
command list:
    dump     display memory in hex and/or ascii modes
    write    write data to specified memory address
	set      set a memory region

See 'mem <command> -h' for more information on a specific command.

generic options:
    -a <memory>    start memory address
	-w <width>     unit size (1/2/4 bytes)
	-h             this help

specific dump options :
    -s <bytes>     default is sector size

specific write options :
	-a <address> <value list>
                   write the value list to memory from <address> in unit specified by -w

#endif
}

int main(int argc, char *argv[])
{
	int i = 1, j;
	int ch;
	char *p = (char *)0, bp[16];
	char *parg;

	while ((ch = getopt(argc, argv, "a:n:h", &parg)) != -1)
	{
		switch(ch)
		{
		case 'a':
			if ((string2value(parg, (u32 *)&p)) < 0)
			{
				printf("Invaild argument\n");

				return -EINVAL;
			}

			break;

		case 'n':
			if ((string2value(parg, (u32 *)&i)) < 0)
			{
				printf("Invaild argument\n");
				printf("%s\n", MD_USAGE);

				return -EINVAL;
			}
			break;

		case 'h':
		default:
			printf("%s\n", MD_USAGE);
			return -EINVAL;
		}
	}

	i = (i + 16) >> 4;
	j = 0;
	while (i--)
	{
		int k;

		memcpy(bp ,p , 16);
		for (k = 0; k < 16; k++)
		{
			if (bp[k] < 0x20 || bp[k] > 0x7e)
				bp[k] = '.';
		}

		printf( "0x%08x: %02x %02x %02x %02x %02x %02x %02x %02x"
			"  %02x %02x %02x %02x %02x %02x %02x %02x  "
			"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
			p, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
			bp[0], bp[1], bp[2], bp[3], bp[4], bp[5], bp[6], bp[7],
			bp[8], bp[9], bp[10], bp[11], bp[12], bp[13], bp[14], bp[15]);

		j++;
		if (33 == j)
		{
			putchar('\n');
			j = 0;
		}

		p += 16;
	}

	return 0;
}

/******************** original mw.c **************************
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
******************** end of mw.c **************************/
