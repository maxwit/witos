#include <getopt.h>
#include <net/net.h>
#include <net/tftp.h>
#include <flash/flash.h>

#define PORT_LEN 6
#define IS_ALPHBIT(c) (((c) >= 'a' && (c) <= 'z') || \
			((c) >= 'A' && (c) <= 'Z'))

struct sub_cmd_info {
	const char *name;
	int (*cmd)(int, char **);
};

static int tftp_get_file(int argc, char **argv);
static int tftp_put_file(int argc, char **argv);

static int port_atoi(const char *str)
{
	const char *iter;
	int interger = 0;
	int temp;

	for (iter = str; *iter >= '0' && *iter <= '9'; iter++) {
		temp = *iter - '0';
		interger = interger * 10 + temp;
	}

	return interger;
}

static int explain_url(const char *buf, struct tftp_opt *dlopt)
{
	__u32 ip;
	int ret;
	int i = 0;
	int port;
	char tmp[FILE_NAME_SIZE + IPV4_STR_LEN + PORT_LEN];

	ip = dlopt->server_ip;
	while (*buf != ':' && *buf != '/' && *buf) {
		tmp[i++] = *buf++;
	}
	tmp[i] = '\0';
	ret = str_to_ip((__u8 *)&dlopt->server_ip, tmp);
	if (ret < 0 && (*buf == ':' || *buf == '/')) {
		while(*buf) {
			tmp[i++] = *buf++;
		}
		tmp[i] = '\0';
		strncpy(dlopt->file_name, tmp, i);
		dlopt->server_ip = ip;
		goto L1;
	} else if (ret < 0 && *buf == '\0') {
		if (i > FILE_NAME_SIZE) {
			return ret;
		}
		strncpy(dlopt->file_name, tmp, i);
		dlopt->server_ip = ip;
		goto L1;
	}
	if (*buf == '\0')
		goto L1;

	if (*buf == ':') {
		buf++;
	}

	i = 0;
	while (*buf != '/' && *buf) {
		if (*buf >= '0' && *buf <= '9') {
			tmp[i++] = *buf++;
			if (i == PORT_LEN) {
				// fixme
				printf("port length too long\n");
				return -1;
			}
		} else {
			// fixme
			printf("Error, the port must be num\n");
			return -1;
		}
	}
	tmp[i] = '\0';
	buf++;
	port = port_atoi(tmp);
	if (port == 0) {
		// fixme
	}

	i = 0;
	while (*buf) {
		tmp[i++] = *buf++;
		if (i >= FILE_NAME_SIZE)
			return -1;
	}
	tmp[i] = '\0';
	strncpy(dlopt->file_name, tmp, i);
L1:
	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	int ret;
	struct sub_cmd_info cmd_info[] = {
		{
			.name = "get",
			.cmd   = tftp_get_file,
		},
		{
			.name = "put",
			.cmd   = tftp_put_file,
		},
	};

	if (argc >= 2) {
		for (i = 0; i < ARRAY_ELEM_NUM(cmd_info); i++) {
			if (0 == strcmp(argv[1], cmd_info[i].name)) {
				ret = cmd_info[i].cmd(argc - 1, argv + 1);
				return ret;
			}
		}
	}

	ret = tftp_get_file(argc, argv);

	return ret;
}
static int tftp_get_file(int argc, char **argv)
{
	int ret, ch;
	char vol;
	bool mem_only = false;
	struct tftp_opt dlopt;
	struct block_device *cur_bdev = NULL;
	char conf_attr[CONF_ATTR_LEN], conf_val[CONF_VAL_LEN];

	memset(&dlopt, 0x0, sizeof(dlopt));
	net_get_server_ip(&dlopt.server_ip);

	while ((ch = getopt(argc, argv, "a:m:r:l:t:v:")) != -1) {
		switch(ch) {
		case 'a':
			ret = str_to_val(optarg, (unsigned long *)&dlopt.load_addr);
			if (ret < 0) {
				usage();
				return ret;
			}

			mem_only = true;
			break;

		case 'm':
			if (strcmp(optarg, "octet") == 0 || strcmp(optarg, "netacsii") == 0) {
				strncpy(dlopt.mode, optarg, MAX_MODE_LEN);
				dlopt.mode[MAX_MODE_LEN - 1] = '\0';
			} else {
				printf("Invalid mode: %s\n", optarg);
				return -EINVAL;
			}
			break;

		case 'l':
			if (IS_ALPHBIT(optarg[0]) && \
			('\0' == optarg[1] || (':' == optarg[1] && '\0' == optarg[2]))) {
				strncpy(dlopt.path, optarg, MAX_PATH_LEN);
				dlopt.path[MAX_PATH_LEN - 1] = '\0';
			} else {
				usage();
				return -EINVAL;
			};
			break;
		case 'r':
			ret = explain_url(optarg, &dlopt);
			if (ret < 0) {
				usage();
				return -ret;
			}
			break;

		case 't':
			dlopt.type = optarg;
			break;

		case 'v':
			dlopt.verbose = true;
			break;


		case 'h':
		default:
			usage();
			return -EINVAL;
		}
	}

	if (mem_only == false) {
		if (dlopt.type) {
			// get_bdev_by_type()?
			cur_bdev = get_bdev_by_volume('A');
		} else {
			if (dlopt.path[0]) {
				vol = dlopt.path[0];
			} else {
				vol = get_curr_volume();
			}

			cur_bdev = get_bdev_by_volume(vol);
		}

		dlopt.bdev = cur_bdev;
		dlopt.type = "jffs2"; // fixme
	}

	if (optind < argc) {
		if (optind + 1 == argc && !dlopt.file_name[0]) {
			strncpy(dlopt.file_name, argv[optind], FILE_NAME_SIZE);
			dlopt.file_name[FILE_NAME_SIZE - 1] = '\0';
		} else {
			usage();
			return -EINVAL;
		}
	}

	if (!dlopt.file_name[0]) {
		snprintf(conf_attr, CONF_ATTR_LEN, "bdev.%s.image.name", dlopt.bdev->name);
		if (conf_get_attr(conf_attr, conf_val) < 0) {
			printf("Please sepcify the filename!\n");
			return -EINVAL;
		}

		strncpy(dlopt.file_name, conf_val, sizeof(dlopt.file_name));
	}

	ret = tftp_download(&dlopt);
	if (ret < 0) {
		printf("fail to download \"%s\" (ret = %d)!\n", dlopt.file_name, ret);
		return ret;
	}

	if (dlopt.bdev) {
		// set file name
		snprintf(conf_attr, CONF_ATTR_LEN, "bdev.%s.image.name", dlopt.bdev->name);
		if (conf_set_attr(conf_attr, dlopt.file_name) < 0) {
			conf_add_attr(conf_attr, dlopt.file_name);
		}

		// set file size
		snprintf(conf_attr, CONF_ATTR_LEN, "bdev.%s.image.size", dlopt.bdev->name);
		val_to_dec_str(conf_val, dlopt.xmit_size);
		if (conf_set_attr(conf_attr, conf_val) < 0) {
			conf_add_attr(conf_attr, conf_val);
		}
	}

	if (false == mem_only)
		conf_store();

	return ret;
}

static int tftp_put_file(int argc, char **argv)
{
	int ret, ch;
	struct tftp_opt opt;
#if 0
	struct block_device *cur_bdev;
#endif

	memset(&opt, 0x0, sizeof(opt));
	net_get_server_ip(&opt.server_ip);

	// fixme
	while ((ch = getopt(argc, argv, "a:m:r:l:t:v:")) != -1) {
		switch(ch) {
		case 'a':
			ret = str_to_val(optarg, (unsigned long *)&opt.load_addr);

			if (ret < 0) {
				usage();
				return ret;
			}
			break;

		case 'm':
			if (strcmp(optarg, "octet") == 0 || strcmp(optarg, "netacsii") == 0) {
				strncpy(opt.mode, optarg, MAX_MODE_LEN);
				opt.mode[MAX_MODE_LEN - 1] = '\0';
			} else {
				printf("Invalid mode: %s\n", optarg);
				return -EINVAL;
			}
			break;

		case 'l':
			if (IS_ALPHBIT(optarg[0]) && \
			('\0' == optarg[1] || (':' == optarg[1] && '\0' == optarg[2]))) {
				strncpy(opt.path, optarg, MAX_PATH_LEN);
				opt.path[MAX_PATH_LEN - 1] = '\0';
			} else {
				usage();
				return -EINVAL;
			};
			break;

		case 'r':
			ret = explain_url(optarg, &opt);
			if (ret < 0) {
				usage();
				return -ret;
			}
			break;

		case 't':
			opt.type = optarg;
			break;

		case 'v':
			opt.verbose = true;
			break;


		case 'h':
		default:
			usage();
			return -EINVAL;
		}
	}

	if (optind < argc) {
		if (optind + 1 == argc && !opt.file_name[0]) {
			strncpy(opt.file_name, argv[optind], FILE_NAME_SIZE);
			opt.file_name[FILE_NAME_SIZE - 1] = '\0';
		} else {
			usage();
			return -EINVAL;
		}
	}

	// fixme
	if (!opt.file_name[0]) {
#if 1
		strcpy(opt.file_name, "a");
#else
		get_default_file_name(cur_bdev, opt->file_name);
#endif
	}

	ret = tftp_upload(&opt);

	return ret;
}
