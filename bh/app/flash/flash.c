#include <flash/flash.h>
#include <flash/part.h>
#include <getopt.h>
#include <sysconf.h>
#include <string.h>
#include <bar.h>
#include <app.h>

static void flash_cmd_usage(char *cmd)
{
	// TODO:
	printf("Usage: flash %s [options <value>]\n"
			"\noptions:\n", cmd);

	if (0 == strcmp(cmd, "scanbb"))
	{
		printf("  -p\t\tset partition number\n"
				"  -h\t\thelp message\n");
		return;
	}

	printf("  -a\t\tset start address\n"
			"  -l\t\tset size of data\n"
			"  -p\t\tset partition number\n"
			"  -h\t\thelp message\n");

	if (0 == strcmp(cmd, "erase"))
	{
		printf("  -f\t\tfoce erase allow bad block\n"
				"  -c\t\t set clean mark\n");
	}
	else if (0 == strcmp(cmd, "read") || 0 == strcmp(cmd, "write"))
	{
		printf("  -m\t\tset memory address\n");
	}
}

static int flash_str_to_val(char * str, u32 * val, char *unit)
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

	return string2value(str, val);
}

static int dump(int argc, char *argv[])
{
	u8  *p, *buff;
	int ch;
	char *optarg;
	int ret   = 0;
	int flag  = 0;
	u32 start = 0;
	int size  = 0;
	char start_unit = 0;
	char size_unit  = 0;
	u32  part_num = PART_CURR;
	struct flash_chip *flash;
	struct partition *part;

	while ((ch = getopt(argc, argv, "p:a:l:h", &optarg)) != -1)
	{
		switch (ch)
		{
		case 'a':
			if (flag || (flash_str_to_val(optarg, &start, &start_unit) < 0))
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);
				return -EINVAL;
			}

			flag = 1;

			break;

		case 'l':
			if (flag == 2 || flash_str_to_val(optarg, (u32 *)&size, &size_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);
				return -EINVAL;
			}

			break;

		case 'p':
			if (flag || string2value(optarg, &part_num) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);
				return -EINVAL;
			}

			flag = 2;

			break;

		default:
			ret = -EINVAL;
		case 'h':
			flash_cmd_usage(argv[0]);
			return ret;
		}
	}

	part = part_open(part_num, OP_RDONLY);
	BUG_ON(NULL == part);

	flash = part->host;
	BUG_ON(NULL == flash);

	// -a xxxblock or -a xxxpage
	if (start_unit == 'b')
	{
		start *= flash->block_size;
	}
	else if (start_unit == 'p')
	{
		start *= flash->page_size;
	}

	// -l xxxblock or -l xxxpage
	if (size_unit == 'b')
	{
		size *= flash->block_size;
	}
	else if (size_unit == 'p')
	{
		size *= flash->page_size;
	}

	if (size == 0)
		size = flash->write_size + flash->oob_size;

	ALIGN_UP(size, flash->page_size + flash->oob_size);

	if (start)
	{
		if (start + size >= flash->chip_size)
		{
			printf("Address 0x%08x overflow!\n", start);
			part_close(part);
			return -EINVAL;
		}
	}
	else
	{
		start = part_get_base(part);
	}

	part_close(part);

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

static int read_write(int argc, char *argv[])
{
	int ch, ret, flag = 0;
	u32 start = 0, size = 1024;
	char start_unit = 0, size_unit = 0;
	void *buff = NULL;
	char *optarg;
	struct flash_chip *flash;

	while ((ch = getopt(argc, argv, "a:l:m:h", &optarg)) != -1)
	{
		switch (ch)
		{
		case 'a':
			if (flag || flash_str_to_val(optarg, &start, &start_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);

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
			if (string2value(optarg, (u32 *)&buff) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);
				return -EINVAL;
			}

			flag++;

			break;

		default:
			ret = -EINVAL;
		case 'h':
			flash_cmd_usage(argv[0]);
			return ret;
		}
	}

	// must set -a flash_addr -l size -m mem_addr
	if (flag != 3)
	{
		printf("Please set the option -a addr -l size -m address>\n");
		flash_cmd_usage(argv[0]);
		return -EINVAL;
	}

	flash = flash_open(BOOT_FLASH_ID);

	// -a xxxblock or -a xxxpage
	if (start_unit == 'b')
	{
		start *= flash->block_size;
	}
	else if (start_unit == 'p')
	{
		start *= flash->page_size;
	}

	// -l xxxblock or -l xxxpage
	if (size_unit == 'b')
	{
		size *= flash->block_size;
	}
	else if (size_unit == 'p')
	{
		size *= flash->page_size;
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
			flash_cmd_usage(argv[0]);

			ret = -EINVAL;
			goto ERROR;
		}
		printf("Read 0x%08x bytes data to mem 0x%08x from flash 0x%08x\n", size, (u32)buff, start);
	}
	else
	{
		ret = flash_write(flash, buff, size,start);
		if (ret < 0)
		{
			printf("please check argument!\n");
			flash_cmd_usage(argv[0]);

			ret = -EINVAL;
			goto ERROR;
		}
		printf("write 0x%08x bytes data to flash 0x%08x from mem 0x%08x\n", size, start, (u32)buff);
	}
ERROR:
	flash_close(flash);

	return ret;
}

// fixme: bad logic!
static int erase(int argc, char *argv[])
{
	int ch;
	char *optarg;
	int arg_flag = 0;
	int ret      = 0;
	u32 start    = 0;
	u32 size     = 0;
	u32 part_num = PART_CURR;
	char start_unit = 0;
	char size_unit  = 0;
	struct flash_chip *flash   = NULL;
	struct partition *pCurPart = NULL;
	u32 erase_flags = EDF_NORMAL;

	if (argc == 1)
	{
		flash_cmd_usage(argv[0]);
		return -EINVAL;
	}

	while ((ch = getopt(argc, argv, "a:l:p::c:f", &optarg)) != -1)
	{
		switch(ch)
		{
		case 'a':
			if (arg_flag == 2 || flash_str_to_val(optarg, &start, &start_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);
				return -EINVAL;
			}

			arg_flag = 1;

			break;

		case 'l':
			if (arg_flag == 2 || flash_str_to_val(optarg, &size, &size_unit) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);
				return -EINVAL;
			}

			break;

		case 'p':
			if (arg_flag == 1 || (optarg && string2value(optarg, &part_num) < 0))
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				flash_cmd_usage(argv[0]);
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
			flash_cmd_usage(argv[0]);
			return ret;
		}
	}

	pCurPart = part_open(part_num, OP_RDWR);
	BUG_ON(NULL == pCurPart);

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
			start *= flash->block_size;
		}
		else if (start_unit == 'p')
		{
			start *= flash->page_size;
		}

		// -l xxxblock or -l xxxpage
		if (size_unit == 'b')
		{
			size *= flash->block_size;
		}
		else if (size_unit == 'p')
		{
			size *= flash->page_size;
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
	ALIGN_UP(size, flash->block_size);

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
static void flash_parterase_usage(void)
{
	printf("Usage: flash parterase <subcommands> [options <value>] [flags]\n"
			"options:\n"
			"  -p   \t\tset partition\n"
			"  -d   \t\tignore bad block\n"
//			"  -j   \t\tJffs2 file system.\n"
		  );
}

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
	u32 size, start;
	u32 flags = EDF_NORMAL;
	struct flash_chip *flash;
	struct partition *part = NULL;
	u32 ch;
	struct flash_parterase_param erInfo;
	char *optarg;
	BOOL need_save = TRUE, for_jffs2 = FALSE;
	FLASH_CALLBACK callback;

	while ((ch = getopt(argc, argv, "p:d", &optarg)) != -1)
	{
		switch (ch)
		{
		case 'p':
			if(string2value(optarg, (u32 *)&nDevNum) < 0)
			{
				flash_parterase_usage();
				goto L1;
			}

			part = part_open(nDevNum, OP_RDWR);
			if (NULL == part)
			{
				flash_parterase_usage();
				goto L1;
			}

			break;

		case 'd':
			flags |= EDF_ALLOWBB;
			break;

		case 'j':
			for_jffs2 = TRUE;
			break;

		case 's':
			need_save = FALSE;
			break;

		default:
			flash_parterase_usage();
			goto L1;
		}
	}

	if (NULL == part)
	{
		part = part_open(PART_CURR, OP_RDWR);

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
	u32 part_num = -1;
	int ch;
	char *optarg;

	while ((ch = getopt(argc, argv, "p:", &optarg)) != -1)
	{
		switch (ch)
		{
		case 'p':
			if (string2value(optarg, &part_num) < 0)
			{
				printf("Invalid argument: \"%s\"\n", optarg);
				return -EINVAL;
			}

			break;

		default:
			flash_cmd_usage(argv[0]);
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

static void flash_usage(void)
{
	printf("Usage: flash command [options <value>]\n"
			"command:\n"
			"  dump      \tview one page\n"
			"  erase     \terase flash\n"
			"  parterase \terase flash partition\n"
			"  partshow  \tshow flash partition\n"
			"  scanbb    \tscan flash bad block\n"
			"  load      \tscan flash data to mem or display\n"
//			"  -j   \t\tJffs2 file system.\n"
			);
}

int main(int argc, char *argv[])
{
	int i;

	struct cmd_info command[] =
	{
		{
			.name = "dump",
			.cmd  = dump
		},
		{
			.name = "erase",
			.cmd  = erase
		},
		{
			.name = "read",
			.cmd  = read_write
		},
		{
			.name = "write",
			.cmd  = read_write
		},
		{
			.name = "scanbb",
			.cmd  = scanbb
		},
	};

	if (argc >= 2)
	{
		for (i = 0; i < ARRAY_ELEM_NUM(command); i++)
		{
			if (0 == strcmp(argv[1], command[i].name))
			{
				command[i].cmd(argc - 1, argv + 1);
				return 0;
			}
		}
	}

	flash_usage();
	return 0;
}
