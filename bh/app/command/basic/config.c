// fix the wolrd!
#include <sysconf.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
	int ret, opt;
	bool reset_all = false;

	usage();

	while ((opt = getopt(argc, argv, "r:h")) != -1)
	{
		switch (opt)
		{
		case 'r':
			if (!strcmp(optarg, "all"))
			{
				reset_all = true;
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

		case 'l':
			conf_list_attr();
			break;

		default:
			usage();
			return -EINVAL;
		}
	}

	ret = conf_store();

	return ret;
}
