// fix the wolrd!
#include <sysconf.h>
#include <getopt.h>

#define LEN 128

static int exec_conf(int option, char *arg, char *val)
{
	char buff[LEN];
	int ret;

	switch (option) {
	case 'a':
		ret = conf_add_attr(arg, val);
		if (ret >= 0)
			printf("add %s successfully\n", arg);

		break;

	case 'd':
		ret = conf_del_attr(arg);
		if (ret >= 0)
			printf("delete %s successfully\n", arg);

		break;

	case 'g':
		ret = conf_get_attr(arg, buff);
		if (!ret)
			printf("%s = %s\n", arg, buff);

		break;

	case 'l':
		ret = conf_list_attr();

		break;

	case 's':
		ret = conf_set_attr(arg, val);
		if (ret >= 0)
			printf("set %s successfully\n", arg);

		break;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int opt, option = 0;
	int ret = 0;
	char *val = NULL;
	char *arg = NULL;

	if (argc == 1) {
		usage();
		return -EINVAL;
	}

	while ((opt = getopt(argc, argv, "a:g:d:s:hl")) != -1)
	{
		switch (opt) {
		case 'l':
		case 'd':
		case 'g':
			if (optind < argc) {
				usage();
				return -EINVAL;
			}

			break;

		case 'a':
		case 's':
			if (optind < argc - 1) {
				usage();
				return -EINVAL;
			}

			break;

		case 'h':
			usage();
			return ret;

		default:
			usage();
			return -EINVAL;
		}

		option = opt;
		arg = optarg;
	}

	if (!option)
		return -EINVAL;

	val = argv[optind];
	ret = exec_conf(option, arg, val);
	if (ret < 0)
		return -EINVAL;

	if (option != 'l' && option != 'g' && option != 'h') {
		ret = conf_store();
		if (ret < 0)
			printf("\nconfig_store faild\n");
	}

	return ret;
}
