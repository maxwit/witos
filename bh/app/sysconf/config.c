// fix the wolrd!
#include <sysconf.h>
#include <getopt.h>

static void usage(void)
{
#if 0
Usage: config [options] [<attribute> [<value>]]
g-bios system configuration utility.

options:
    -a <attr> <value>     adds a new attribue with value
    -r <attr>             removes the attribute
    -g <attr>             get value
    -l                    list all
    -h                    this help

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

	usage();

	while ((opt = getopt(argc, argv, "r:h")) != -1)
	{
		switch (opt)
		{
		case 'r':
			if (!strcmp(optarg, "all"))
			{
				reset_all = TRUE;
				printf("reseting all configuration to default!");
			}
			else if (!strcmp(optarg, "net"))
			{
				printf("reseting network configuration to default!");
			}
			else if (!strcmp(optarg, "boot"))
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
