#include <loader.h>
#include <uart/ymodem.h>
#include <uart/kermit.h>
#include <sysconf.h>
#include <getopt.h>

static void usage(int argc, char *argv[])
{
#if 0

Usage: kermit [options]
Load file from uart with kermit, and write to storage or memory only(default to storage).

options:
	-m [ADDR]   load data to memory, but not write to storage.
                if ADDR is not specified, the malloc one
	-h          display this help.

#endif
	printf("Usage: kermit [options] [ADDR] \n"
		"Load file to flash or memory only.(defualt load to flash)\n"
		"\noptions:\n"
		"\t-m : Only load the file to memory(else to flash), and the address is VAL(default file location)\n"
		);
}

int main(int argc, char *argv[])
{
	int size, ret;
	struct loader_opt ld_opt;
	struct partition *part;
	int opt;
	int flag = 0;

	size = 0;
	part = NULL;

	usage(argc, argv);
	return 0;

	memset(&ld_opt, 0x0, sizeof(ld_opt));

	while ((opt = getopt(argc, argv, "m::")) != -1)
	{
		switch (opt)
		{
		case 'm':
			if (optarg != NULL)
			{
				ret = str_to_val(optarg, (__u32 *)&ld_opt.load_addr);
				if (ret < 0)
				{
					printf("Input a invalied address!\n");
					return -EINVAL;
				}
			}
			flag = 1;
			break;

		default:
			break;
		}
	}

	if (flag == 0)
	{
		printf("writing back to partition is not supported yet!\n");
		return -EINVAL;
	}

	if (strcmp(argv[0], "kermit") == 0)
	{
		size = kermit_load(&ld_opt);
	}

	if (strcmp(argv[0], "ymodem") == 0)
	{
		size = ymodem_load(&ld_opt);
	}

	return size;
}
