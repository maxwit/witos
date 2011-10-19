// fix the wolrd!
#include <sysconf.h>
#include <getopt.h>

static void usage(void)
{
#if 0
Usage: config [OPTION [VAL]]
Operate system configuration information!\n

OPTION:
	-r <all|net|boot>: reset system configuration.
	-l : List system configuration information.
	-h : this help message.

#endif
	printf("Usage: config [OPTION [VAL]]\n"
		"Operate system configuration information!\n"
		"\nOPTION:\n"
		"\t-l : List system configuration information!"
		"\t-r [all|net|boot]: reset system configuration(default all).\n"
		"\t-h : this help message.\n");
}

int main(int argc, char *argv[])
{
	int ret, opt;
	BOOL reset_all = FALSE;
	char *arg;

	usage();

	while ((opt = getopt(argc, argv, "r:h", &arg)) != -1)
	{
		switch (opt)
		{
		case 'r':
			if (!strcmp(arg, "all"))
			{
				reset_all = TRUE;
				printf("reseting all configuration to default!");
			}
			else if (!strcmp(arg, "net"))
			{
				printf("reseting network configuration to default!");
			}
			else if (!strcmp(arg, "boot"))
			{
				printf("reseting Linux configuration to default!");
			}
			else
			{
				usage();
				return -EINVAL;
			}
			break;

		default:
			usage();
			return -EINVAL;
		}
	}

	ret = sysconf_save();

	return ret;
}
