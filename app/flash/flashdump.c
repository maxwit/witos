#include <g-bios.h>
#include <flash/flash.h>
#include <flash/part.h>
#include <getopt.h>
#include <flash/part.h>


static void dump_usage(void)
{
	printf("Usage: flashdump [options <value>]\n"
		"\noptions:\n"
		"  -b\tset flash block number\n"
		"  -p\tset flash page number\n"
		"  -a\tset flash address\n");
}


int main(int argc, char *argv[])
{
	int ch, ret, flag;
	u8  *p, *buff;
	u32 start, size;
	struct flash_chip *flash;
	struct partition *curr_part;
	char *optarg;


	// fixme: flash location
	curr_part = part_open(PART_CURR, OP_RDONLY);
	BUG_ON(NULL == curr_part);

	flash = curr_part->host;
	BUG_ON(NULL == flash);

	size  = flash->write_size + flash->oob_size;
	start = part_get_base(curr_part);

	part_close(curr_part);

	flag = 0;
	while ((ch = getopt(argc, argv, "b:p:a:h", &optarg)) != -1)
	{
		switch (ch)
		{
		case 'b':
			if (flag || string2value(optarg, &start) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				dump_usage();

				return -EINVAL;
			}

			start = start * flash->block_size;

			if (start >= flash->chip_size)
			{
				printf("Block number 0x%08x overflow!\n", start);
				return -EINVAL;
			}

			flag = 1;

			break;

		case 'a':
			if (flag || string2value(optarg, &start) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);

				dump_usage();

				return -EINVAL;
			}

			if (start >= flash->chip_size)
			{
				printf("Address 0x%08x overflow!\n", start);
				return -EINVAL;
			}

			flag = 1;

			break;

		case 'p':
			if (flag || string2value(optarg, &start) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				dump_usage();

				return -EINVAL;
			}

			start = start * flash->page_size;

			if (start >= flash->chip_size)
			{
				printf("Page number 0x%08x overflow!\n", start);
				return -EINVAL;
			}

			flag = 1;

			break;

		case 'h':
			dump_usage();
			return 0;

		default:
			dump_usage();
			return -EINVAL;
		}
	}


	buff = (u8 *)malloc(size);
	if (NULL == buff)
	{
		return -ENOMEM;
	}

	start &= ~(flash->write_size - 1);

	ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_RAW);
	// if ret < 0
	ret = flash_read(flash, buff, start, size);
	if (ret < 0)
	{
		printf("%s(): line %d execute flash_read_raw() error!\n"
			"error = %d\n", __FUNCTION__, __LINE__, ret);

		free(buff);

		return ret;
	}

	DPRINT("Flash 0x%08x ==> RAM 0x%08x, Expected length 0x%08x, Real length 0x%08x\n\n",
		   start, buff, size, ret);

	p = buff;

	while (size > 0)
	{
		int i;

		printf("Page @ 0x%08x:\n", start);

		i = flash->write_size >> 4;

		while (i--)
		{
			printf( "\t%02x %02x %02x %02x %02x %02x %02x %02x"
				"  %02x %02x %02x %02x %02x %02x %02x %02x\n",
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);

			p += 16;
		}

		printf("OOB @ 0x%08x + 0x08x:\n", start, flash->write_size);

		i = flash->oob_size >> 4;

		while (i--)
		{
			printf( "\t%02x %02x %02x %02x %02x %02x %02x %02x"
				"  %02x %02x %02x %02x %02x %02x %02x %02x\n",
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);

			p += 16;
		}

		start += flash->write_size + flash->oob_size;
		size  -= flash->write_size + flash->oob_size;

		puts("\n");
	}

	free(buff);

	flash_close(flash);

	return 0;
}

