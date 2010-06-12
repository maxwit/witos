#include <g-bios.h>

#define MD_USAGE 			"Usage md <ram address> [number]"
#define MEMZERO_USAGE		"Usage memzero <ram address> [number]. Carefully use it!!!"


int main(int argc, char *argv[])
{
	int i, j;
	char *p, bp[16];

	switch(argc)
	{
	case 1:
		i = 528 >> 4;
		p = (char *)0;
		break;

	case 2:
		i = 528 >> 4;
		string2value(argv[1], (u32 *)&p);
		break;

	case 3:
		string2value(argv[1], (u32 *)&p);
		string2value(argv[2], (u32 *)&i);
		i = (i * 528) >> 4;
		break;

	default:
		printf("%s\n", MD_USAGE);
		return -EINVAL;
	}

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


