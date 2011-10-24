#include <flash/flash.h>
#include <getopt.h>
#include <sysconf.h>
#include <string.h>
#include <bar.h>
#include <app.h>

static int flash_str_to_val(char * str, __u32 * val, char *unit)
{
	int len;
	char *p = str;

	len = strlen(str);

	if (len > 5 && 0 == strncmp(p + len - 5, "block", 5))
	{
		*unit = 'b'; // block
		p[len - 5] = '\0';
	}
	else if (len > 4 && 0 == strncmp(p + len - 4, "page", 4))
	{
		*unit = 'p'; // page
		p[len - 4] = '\0';
	}
	else if (len > 1 && strchr("kKmMgG", str[len - 1]))
	{
		return hr_str_to_val(str, val);
	}

	return str_to_val(str, val);
}
#if 0

static int read_write(int argc, char *argv[])
{
	int ch, ret, flag = 0;
	__u32 start = 0, size = 1024;
	char start_unit = 0, size_unit = 0;
	void *buff = NULL;
	struct flash_chip *flash;

	while ((ch = getopt(argc, argv, "a:l:m:h")) != -1)
	{
		switch (ch)
		{
		case 'a':
			if (flag || flash_str_to_val(optarg, &start, &start_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();

				return -EINVAL;
			}

			flag++;

			break;

		case 'l':
			if (flash_str_to_val(optarg, &size, &size_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				return -EINVAL;
			}

			flag++;

			break;

		case 'm':
			if (str_to_val(optarg, (__u32 *)&buff) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			flag++;

			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	// must set -a flash_addr -l size -m mem_addr
	if (flag != 3)
	{
		printf("Please set the option -a addr -l size -m address>\n");
		usage();
		return -EINVAL;
	}

	flash = flash_open(BOOT_FLASH_ID);

	// -a xxxblock or -a xxxpage
	if (start_unit == 'b')
	{
		start *= flash->erase_size;
	}
	else if (start_unit == 'p')
	{
		start *= flash->write_size;
	}

	// -l xxxblock or -l xxxpage
	if (size_unit == 'b')
	{
		size *= flash->erase_size;
	}
	else if (size_unit == 'p')
	{
		size *= flash->write_size;
	}

	if (start + size >= flash->chip_size)
	{
		printf("Address 0x%08x overflow!\n", start + size);
		ret = -EINVAL;
		goto ERROR;
	}

	if (0 == strcmp(argv[0], "read"))
	{
		ret = flash_read(flash, buff, start, size);
		if (ret < 0)
		{
			printf("please check argument!\n");
			usage();

			ret = -EINVAL;
			goto ERROR;
		}
		printf("Read 0x%08x bytes data to mem 0x%08x from flash 0x%08x\n", size, (__u32)buff, start);
	}
	else
	{
		ret = flash_write(flash, buff, size,start);
		if (ret < 0)
		{
			printf("please check argument!\n");
			usage();

			ret = -EINVAL;
			goto ERROR;
		}
		printf("write 0x%08x bytes data to flash 0x%08x from mem 0x%08x\n", size, start, (__u32)buff);
	}
ERROR:
	flash_close(flash);

	return ret;
}

// fixme: bad logic!
static int erase(int argc, char *argv[])
{
	int ch;
	int arg_flag = 0;
	int ret      = 0;
	__u32 start    = 0;
	__u32 size     = 0;
	__u32 part_num = PART_CURR;
	char start_unit = 0;
	char size_unit  = 0;
	struct flash_chip *flash   = NULL;
	struct partition *pCurPart = NULL;
	__u32 erase_flags = EDF_NORMAL;

	if (argc == 1)
	{
		usage();
		return -EINVAL;
	}

	while ((ch = getopt(argc, argv, "a:l:p::c:f")) != -1)
	{
		switch(ch)
		{
		case 'a':
			if (arg_flag == 2 || flash_str_to_val(optarg, &start, &start_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			arg_flag = 1;

			break;

		case 'l':
			if (arg_flag == 2 || flash_str_to_val(optarg, &size, &size_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			break;

		case 'p':
			if (arg_flag == 1 || (optarg && str_to_val(optarg, &part_num) < 0))
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			arg_flag = 2;

			break;

		case 'c':
			erase_flags |= EDF_JFFS2;
			break;

		case 'f':
			erase_flags |= EDF_ALLOWBB;
			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	pCurPart = flash_bdev_open(part_num, OP_RDWR);
	assert(pCurPart);

	flash = pCurPart->host;

	// if option is "-p" erase the whole partiton or default current partition with no option
	if (arg_flag == 2)
	{
		start = pCurPart->attr->part_base;
		size  = pCurPart->attr->part_size;
	}
	else
	{
		// -a xxxblock or -a xxxpage
		if (start_unit == 'b')
		{
			start *= flash->erase_size;
		}
		else if (start_unit == 'p')
		{
			start *= flash->write_size;
		}

		// -l xxxblock or -l xxxpage
		if (size_unit == 'b')
		{
			size *= flash->erase_size;
		}
		else if (size_unit == 'p')
		{
			size *= flash->write_size;
		}
	}

	part_close(pCurPart);

	if (flash->chip_size < start + size)
	{
		printf("Out of chip size!\n");
		return -EINVAL;
	}

	//aligned:
	ALIGN_UP(start, flash->write_size);
	ALIGN_UP(size, flash->erase_size);

	printf("[0x%08x : 0x%08x]\n", start, size);
	ret = flash_erase(flash, start, size, erase_flags);

	return ret;
}

/* part erase */
struct flash_parterase_param
{
	FLASH_HOOK_PARAM hParam;
	struct process_bar *pBar;
};

#if 0
static int flash_parterase_process(struct flash_chip *flash, FLASH_HOOK_PARAM *pParam)
{
	struct flash_parterase_param *pErInfo;

	pErInfo = container_of(pParam, struct flash_parterase_param, hParam);

	progress_bar_set_val(pErInfo->pBar, pParam->nBlockIndex);

	return 0;
}

static int parterase(int argc, char *argv[])
{
	int ret = 0, nDevNum;
	__u32 size, start;
	__u32 flags = EDF_NORMAL;
	struct flash_chip *flash;
	struct partition *part = NULL;
	__u32 ch;
	struct flash_parterase_param erInfo;
	bool need_save = true, for_jffs2 = false;
	FLASH_CALLBACK callback;

	while ((ch = getopt(argc, argv, "p:d")) != -1)
	{
		switch (ch)
		{
		case 'p':
			if(str_to_val(optarg, (__u32 *)&nDevNum) < 0)
			{
				usage();
				goto L1;
			}

			part = flash_bdev_open(nDevNum, OP_RDWR);
			if (NULL == part)
			{
				usage();
				goto L1;
			}

			break;

		case 'd':
			flags |= EDF_ALLOWBB;
			break;

		case 'j':
			for_jffs2 = true;
			break;

		case 's':
			need_save = false;
			break;

		default:
			usage();
			goto L1;
		}
	}

	if (NULL == part)
	{
		part = flash_bdev_open(PART_CURR, OP_RDWR);

		if (NULL == part)
		{
			printf("fail to open current partition!\n");
			ret = -EACCES;
			goto L1;
		}
	}

	if (for_jffs2)
	{
		if (part->attr->part_type == PT_FS_JFFS2)
			flags |= EDF_JFFS2;
		else
			printf("not JFFS2 part (-j skipped)\n");
	}

	flash = part->host;

	start = part_get_base(part);
	size  = part_get_size(part);

	create_progress_bar(&erInfo.pBar,
				start >> flash->erase_shift,
				(start + size - 1) >> flash->erase_shift
				);

	callback.func = flash_parterase_process;
	callback.args = &erInfo.hParam;
	flash_ioctl(flash, FLASH_IOCS_CALLBACK, &callback);

	ret = flash_erase(flash, start, size, flags);
	printf("\n");

	delete_progress_bar(erInfo.pBar);

	part_set_image(part, "", 0);

	if (need_save)
	{
		ret = sysconf_save();
		if (ret < 0)
			printf("Can not save configure information to flash");
	}

	part_close(part);

L1:
	return ret;
}
#endif

/* scan bb */
// TODO: add process bar
static int scanbb(int argc, char *argv[])
{
	int ret;
	struct flash_chip *flash;
	__u32 part_num = -1;
	int ch;

	while ((ch = getopt(argc, argv, "p:")) != -1)
	{
		switch (ch)
		{
		case 'p':
			if (str_to_val(optarg, &part_num) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				return -EINVAL;
			}

			break;

		default:
			usage();
			return -EINVAL;
		}
	}

	flash = flash_open(BOOT_FLASH_ID);
	if (NULL == flash)
	{
		ret = -ENODEV;
		goto L1;
	}

	if (FLASH_NANDFLASH == flash->type)
	{
		// fixme
	}

	flash_ioctl(flash, FLASH_IOC_SCANBB, &part_num);

	flash_close(flash);
L1:
	return ret;
}
#endif

static int dump(int argc, char *argv[])
{
	__u8	*p, *buff;
	int ch;
	int ret   = 0;
	int flag  = 0;
	__u32 start = 0;
	int size  = 0;
	char start_unit = 0;
	char size_unit	= 0;
	struct flash_chip *flash;

	while ((ch = getopt(argc, argv, "p:a:l:h")) != -1)
	{
		switch (ch)
		{
		case 'a':
			if (flag || (flash_str_to_val(optarg, &start, &start_unit) < 0))
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			flag = 1;

			break;

		case 'l':
			if (flag == 2 || flash_str_to_val(optarg, (__u32 *)&size, &size_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			break;


		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	flash = flash_open("mtdblock0");
	assert(flash);

	// -a xxxblock or -a xxxpage
	if (start_unit == 'b')
	{
		start *= flash->erase_size;
	}
	else if (start_unit == 'p')
	{
		start *= flash->write_size;
	}

	// -l xxxblock or -l xxxpage
	if (size_unit == 'b')
	{
		size *= flash->erase_size;
	}
	else if (size_unit == 'p')
	{
		size *= flash->write_size;
	}

	if (size == 0)
		size = flash->write_size + flash->oob_size;

	ALIGN_UP(size, flash->write_size + flash->oob_size);

	if (start)
	{
		if (start + size >= flash->chip_size)
		{
			printf("Address 0x%08x overflow!\n", start);
			flash_close(flash);
			return -EINVAL;
		}
	}
	else
	{
		start = flash->bdev.bdev_base;
	}

	flash_close(flash);

	buff = (__u8 *)malloc(size);
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
			"error = %d\n", __func__, __LINE__, ret);

		goto L1;
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

L1:
	free(buff);

	// flash_close(flash);

	return ret;
}


// fixme: bad logic!
static int erase(int argc, char *argv[])
{
	int ch;
	int arg_flag = 0;
	int ret      = 0;
	__u32 start    = 0;
	__u32 size     = 0;
	__u32 dev_num = 0;
	char start_unit = 0;
	char size_unit  = 0;
	struct flash_chip *flash   = NULL;
	__u32 erase_flags = EDF_NORMAL;

	if (argc == 1)
	{
		usage();
		return -EINVAL;
	}

	while ((ch = getopt(argc, argv, "a:l:p::c:f")) != -1)
	{
		switch(ch)
		{
		case 'a':
			if (arg_flag == 2 || flash_str_to_val(optarg, &start, &start_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			arg_flag = 1;

			break;

		case 'l':
			if (arg_flag == 2 || flash_str_to_val(optarg, &size, &size_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			break;

		case 'p':
			if (arg_flag == 1 || (optarg && str_to_val(optarg, &dev_num) < 0))
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			arg_flag = 2;

			break;

		case 'c':
			erase_flags |= EDF_JFFS2;
			break;

		case 'f':
			erase_flags |= EDF_ALLOWBB;
			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	flash = flash_open("mtdblock0p2");

	// if option is "-p" erase the whole partiton or default current partition with no option
	if (arg_flag == 2)
	{
		start = flash->bdev.bdev_base;
		size  = flash->bdev.bdev_size;
	}
	else
	{
		// -a xxxblock or -a xxxpage
		if (start_unit == 'b')
		{
			start *= flash->erase_size;
		}
		else if (start_unit == 'p')
		{
			start *= flash->write_size;
		}

		// -l xxxblock or -l xxxpage
		if (size_unit == 'b')
		{
			size *= flash->erase_size;
		}
		else if (size_unit == 'p')
		{
			size *= flash->write_size;
		}
	}

	flash_close(flash);

	if (flash->chip_size < start + size)
	{
		printf("Out of chip size!\n");
		return -EINVAL;
	}

	//aligned:
	ALIGN_UP(start, flash->write_size);
	ALIGN_UP(size, flash->erase_size);

	printf("[0x%08x : 0x%08x]\n", start, size);
	ret = flash_erase(flash, start, size, erase_flags);

	return ret;
}


static int read_write(int argc, char *argv[])
{
	return 0;
}

static int scanbb(int argc, char *argv[])
{
	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	usage();
	return 0;

	struct command cmd[] =
	{
		{
			.name = "dump",
			.main = dump
		},
		{
			.name = "erase",
			.main = erase
		},
		{
			.name = "read",
			.main = read_write
		},
		{
			.name = "write",
			.main = read_write
		},
		{
			.name = "scanbb",
			.main = scanbb
		},
	};

	if (argc >= 2)
	{
		for (i = 0; i < ARRAY_ELEM_NUM(cmd); i++)
		{
			if (0 == strcmp(argv[1], cmd[i].name))
			{
				cmd[i].main(argc - 1, argv + 1);
				return 0;
			}
		}
	}

	usage();

	return 0;
}
