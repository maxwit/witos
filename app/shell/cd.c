#include <g-bios.h>
#include <sysconf.h>
#include <string.h>
#include <getopt.h>
#include <flash/part.h>
#include <bar.h>


int main(int argc, char *argv[])
{
	int dev_num;

	switch (argc)
	{
	case 1:
		dev_num = part_get_home(); // fixme: changing to OS root seems  more reasonable
		break;

	case 2:
		if(string2value(argv[1], (u32 *)&dev_num) < 0 || dev_num < 0)
		{
			goto L1;
		}
		break;

	default:
		goto L2;
	}

	if (part_change(dev_num) < 0)
		goto L2;

	return 0;

L1:
	printf("Device \"%s\" is wrong!\n", argv[1]);
L2:
	printf("Usage: cd Device (For example: \"cd 0\").\n");
	return -EINVAL;
}



