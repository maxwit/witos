// fix the wolrd!
#include <sysconf.h>
#include <getopt.h>

static void usage(int argc, char *argv[])
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

	usage(argc, argv);

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
				usage(argc, argv);
				return -EINVAL;
			}
			break;

		case 'l':
			conf_list_attr();
			break;

		default:
			usage(argc, argv);
			return -EINVAL;
		}
	}

	ret = conf_store();

	return ret;
}
