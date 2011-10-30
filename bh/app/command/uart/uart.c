#include <loader.h>
#include <uart/ymodem.h>
#include <uart/kermit.h>
#include <uart/uart.h>
#include <sysconf.h>
#include <getopt.h>
#include <flash/flash.h>

static int uart_load(int argc, char *argv[])
{
	int size, ret;
	struct loader_opt ld_opt;
	char *pro = NULL;
	int opt;

	size = 0;

	memset(&ld_opt, 0x0, sizeof(ld_opt));

	ld_opt.file = get_bdev_by_volume(get_curr_volume())->file;

	while ((opt = getopt(argc, argv, "m::p:f:i:vh")) != -1) {
		switch (opt) {
		case 'm':
			if (optarg != NULL) {
				ret = str_to_val(optarg, (__u32 *)&ld_opt.load_addr);
				if (ret < 0) {
					printf("Input a invalied address!\n");

					return -EINVAL;
				}
			}

			ld_opt.file = NULL;

			break;

		case 'v':
		case 'i':
		case 'f':

			// fixme

			break;

		case 'h':
			usage();

			return 0;

		case 'p':
			pro = optarg;

			break;

		default:
			usage();

			return -EINVAL;
		}
	}

	if (0 == strcmp(pro, "k")) {
		printf("load kermit....:");
		size = kermit_load(&ld_opt);
	} else if (0 == strcmp(pro, "y")) {
		printf("load ymode....:");
		size = ymodem_load(&ld_opt);
	} else {
		usage();
		return -EINVAL;
	}

	if (ld_opt.file && size > 0) {
		strncpy(ld_opt.file->name, ld_opt.file_name, MAX_FILE_NAME_LEN);
		ld_opt.file->size = size;

		set_bdev_file_attr(ld_opt.file);
	}

	return size;
}

static int uart_setup(int argc ,char *argv[])
{
	int opt;
	int flag = 0;
	int ret = 0;

	while ((opt = getopt(argc, argv, "hi:")) != -1) {
		switch (opt) {
		case 'i':
			// fixme
			flag = 1;

			break;

		default:
			ret = -EINVAL;

		case 'h':
			usage();

			return ret;
		}
	}

	if (flag == 1)
	{
		// fixme
	} else {
		usage();
	}

	return 0;
}

static int uart_send(int argc ,char *argv[])
{
	int opt;
	int het;
	int ret = 0;
	struct loader_opt ld_opt;

	memset(&ld_opt, 0x0, sizeof(ld_opt));
	while ((opt = getopt(argc, argv, "m::p:f:i:vh")) != -1) {
		switch (opt) {
		case 'm':
			if (optarg != NULL) {
				het = str_to_val(optarg, (__u32 *)&ld_opt.load_addr);
				if (het < 0) {
					printf("Input a invalied address!\n");

					return -EINVAL;
				}
			}

			break;

		case 'p':
		case 'v':
		case 'i':
		case 'f':

			// fixme

			break;

		default:
			ret = -EINVAL;

		case 'h':
			usage();
			return ret;
		}
	}

	// fixme

	return 0;
}

static int uart_test(int argc ,char *argv[])
{
	int opt;
	int het;
	int ret = 0;
	struct loader_opt ld_opt;

	memset(&ld_opt, 0x0, sizeof(ld_opt));
	while ((opt = getopt(argc, argv, "m::p:f:i:vh")) != -1) {
		switch (opt) {
		case 'm':
			if (optarg != NULL) {
				het = str_to_val(optarg, (__u32 *)&ld_opt.load_addr);
				if (het < 0) {
					printf("Input a invalied address!\n");

					return -EINVAL;
				}
			}

			break;

		case 'p':
		case 'v':
		case 'i':
		case 'f':

			// fixme

			break;

		default:
			ret = -EINVAL;

		case 'h':
			usage();
			return ret;
		}
	}

	// fixme

	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	int size;
	int ret = 0;

	struct command cmd[] = {
		{
			.name = "load",
			.main = uart_load
		},
		{
			.name = "setup",
			.main = uart_setup
		},
		{
			.name = "send",
			.main = uart_send
		},
		{
			.name = "test",
			.main = uart_test
		}
	};

	if (argc >= 2) {
		for (i = 0; i < ARRAY_ELEM_NUM(cmd); i++) {
			if (0 == strcmp(argv[1], cmd[i].name)) {
				size = cmd[i].main(argc - 1, argv +1);

				return size;
			}
		}
		ret = -EINVAL;
	}

	usage();
	return ret;
}
