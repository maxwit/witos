#include <loader.h>
#include <uart/ymodem.h>
#include <uart/kermit.h>
#include <sysconf.h>
#include <getopt.h>

static void usage(int argc, char *argv[])
{
#if 0

Usage: uart <command> [options]
Load file from uart with ymodem, and write to storage or memory only (default to storage).

options:
	-m [ADDR]   load data to memory, but not write to storage.
                if ADDR is not specified, the malloc one
	-h          display this help.

#endif
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

	printf("%s loading ...", argv[0]);

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
#if 0
		if ((part = flash_bdev_open(PART_CURR, OP_RDWR)) == NULL)
		{
			return -EINVAL;
		}

		ld_opt.part = part;
#endif
	}

	if (strcmp(argv[0], "kermit") == 0)
	{
		size = kermit_load(&ld_opt);
	}

	if (strcmp(argv[0], "ymodem") == 0)
	{
		size = ymodem_load(&ld_opt);
	}

#if 0
	if (flag == 0)
	{
		part_set_image(part, ld_opt.file_name, ld_opt.load_size);

		part_close(part);

		sysconf_save();
	}
#endif

	return size;
}
