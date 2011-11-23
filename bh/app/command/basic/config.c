#include <sysconf.h>
#include <getopt.h>

#define LEN 128
// fixme
#define MAX_OPT 5

struct conf_opt {
	int opt;
	const char *attr, *val;
};

static int do_config(int option, const char *arg, const char *val)
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

	default:
		printf("Invalid option '%c'\n", option);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int i, j, opt, ret;
	struct conf_opt opt_list[MAX_OPT];

	if (argc == 1) {
		usage();
		return -EINVAL;
	}

	i = 0;
	while ((opt = getopt(argc, argv, "a:g:d:s:lh")) != -1) {
		switch (opt) {
		case 'd':
		case 'g':
			opt_list[i].attr = optarg;
		case 'l':
			opt_list[i].opt = opt;
			break;

		case 'a':
		case 's':
			if (optind == argc || '-' == argv[optind][0]) {
				usage();
				return -EINVAL;
			}
			opt_list[i].opt  = opt;
			opt_list[i].attr = optarg;
			opt_list[i].val  = argv[optind];
			optind++;
			break;

		case 'h':
			usage();
			return 0;

		default:
			usage();
			return -EINVAL;
		}

		i++;
	}

	for (j = 0; j < i; j++) {
		ret = do_config(opt_list[j].opt, opt_list[j].attr, opt_list[j].val);
		if (ret < 0)
			return -EINVAL;
	}

	ret = conf_store();
	if (ret < 0)
		printf("\nconfig_store faild\n");

	return ret;
}
