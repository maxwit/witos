#include <flash/flash.h>
#include <flash/part.h>
#include <getopt.h>

static void part_usage(void)
{
	printf("\nUsage: part [option]\n"
			"\nOptions:\n"
			"  -l \tshow all partion infomation\n"
			"  -h \tthis help\n"
			);
}

int main(int argc, char *argv[])
{
	int opt;
	u32 num = 0;
	char *arg;
	struct flash_chip *flash;

	if (argc < 2)
	{
		part_usage();
		return -EINVAL;
	}

	while ((opt = getopt(argc, argv, "l::h", &arg)) != -1)
	{
		switch (opt)
		{
		case 'l':
			if (arg == NULL)
			{
				for (num = 0; num < MAX_FLASH_DEVICES; num++)
				{
					flash = flash_open(num);
					if (flash)
					{
						part_show(flash);
						flash_close(flash);
					}
				}
			}
			else
			{
				string2value(arg, &num);
				flash = flash_open(num);
				if (flash)
				{
					part_show(flash);
					flash_close(flash);
				}
			}
			break;
		case 'h':
		default:
			part_usage();
			return -EINVAL;
		}
	}

	return 0;
}
