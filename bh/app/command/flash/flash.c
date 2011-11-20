#include <flash/flash.h>
#include <getopt.h>
#include <sysconf.h>
#include <string.h>
#include <bar.h>
#include <task.h>

static int flash_str_to_val(char * str, __u32 * val, char *unit)
{
	int len;
	char *p = str;

	len = strlen(str);

	if (len > 5 && 0 == strncmp(p + len - 5, "block", 5)) {
		*unit = 'b'; // block
		p[len - 5] = '\0';
	} else if (len > 4 && 0 == strncmp(p + len - 4, "page", 4)) {
		*unit = 'p'; // page
		p[len - 4] = '\0';
	} else if (len > 1 && strchr("kKmMgG", str[len - 1])) {
		return hr_str_to_val(str, val);
	}

	return str_to_val(str, val);
}

static struct flash_chip *get_current_flash()
{
	char v;
	struct block_device *bdev;
	struct flash_chip *flash;

	v = get_curr_volume();
	bdev = get_bdev_by_volume(v);
	flash = container_of(bdev, struct flash_chip, bdev);
	if (0 != strncmp(flash->bdev.dev.name, "mtdblock", strlen("mtdblock"))) {
		printf("The current volume is not a flash device!\n");
		return NULL;
	}

	return flash;
}

static int is_master(struct flash_chip *flash)
{
	if (NULL == strchr(flash->bdev.dev.name, 'p'))
		return 1;

	return 0;
}

static int info(int argc, char *argv[])
{
	struct flash_chip *flash;

	flash = get_current_flash();
	if (flash == NULL) {
		return -ENODEV;
	}

	printf(
		"volume     %c:\n"
		"flash:     %s\n"
		"device:    %s\n"
		"base:      0x%08X\n"
		"size:      0x%08X\n"
		"pagesize:  0x%08X\n"
		"blocksize: 0x%08x\n",
		flash->bdev.volume,
		is_master(flash) ? flash->name : flash->master->name,
		flash->bdev.dev.name,
		flash->bdev.bdev_base,
		flash->bdev.bdev_size,
		flash->write_size,
		flash->erase_size);

	return 0;
}

static int read_write(int argc, char *argv[])
{
	int ch, ret, flag = 0;
	__u32 start = 0, size = 1024;
	char start_unit = 0, size_unit = 0;
	void *buff = NULL;
	struct flash_chip *flash;

	while ((ch = getopt(argc, argv, "a:l:m:h")) != -1) {
		switch (ch) {
		case 'a':
			if (flag || flash_str_to_val(optarg, &start, &start_unit) < 0) {
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();

				return -EINVAL;
			}

			flag++;

			break;

		case 'l':
			if (flash_str_to_val(optarg, &size, &size_unit) < 0) {
				printf("Invalid argument: \"%s\"\n", optarg);
				return -EINVAL;
			}

			flag++;

			break;

		case 'm':
			if (str_to_val(optarg, (__u32 *)&buff) < 0) {
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
	if (flag != 3) {
		printf("Please set the option -a addr -l size -m address>\n");
		usage();
		return -EINVAL;
	}

	flash = get_current_flash();
	assert(flash);

	// -a xxxblock or -a xxxpage
	if (start_unit == 'b') {
		start *= flash->erase_size;
	} else if (start_unit == 'p') {
		start *= flash->write_size;
	}

	// -l xxxblock or -l xxxpage
	if (size_unit == 'b') {
		size *= flash->erase_size;
	} else if (size_unit == 'p') {
		size *= flash->write_size;
	}

	if (start + size >= flash->chip_size) {
		printf("Address 0x%08x overflow!\n", start + size);
		ret = -EINVAL;
		goto ERROR;
	}

	if (0 == strcmp(argv[0], "read")) {
		ret = flash_read(flash, buff, start, size);
		if (ret < 0) {
			printf("please check argument!\n");
			usage();

			ret = -EINVAL;
			goto ERROR;
		}
		printf("Read 0x%08x bytes data to mem 0x%08x from flash 0x%08x\n", size, (__u32)buff, start);
	} else {
		ret = flash_write(flash, buff, size, start);
		if (ret < 0) {
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

#if 0
/* part erase */
struct flash_parterase_param
{
	FLASH_HOOK_PARAM hParam;
	struct process_bar *pBar;
};

static int flash_parterase_process(struct flash_chip *flash, FLASH_HOOK_PARAM *pParam)
{
	struct flash_parterase_param *pErInfo;

	pErInfo = container_of(pParam, struct flash_parterase_param, hParam);

	progress_bar_set_val(pErInfo->pBar, pParam->nBlockIndex);

	return 0;
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

	while ((ch = getopt(argc, argv, "p:")) != -1) {
		switch (ch) {
		case 'p':
			if (str_to_val(optarg, &part_num) < 0) {
				printf("Invalid argument: \"%s\"\n", optarg);
				return -EINVAL;
			}

			break;

		default:
			usage();
			return -EINVAL;
		}
	}

	flash = get_current_flash();
	if (NULL == flash) {
		ret = -ENODEV;
		goto L1;
	}

	if (FLASH_NANDFLASH == flash->type) {
		// fixme
	}

	flash_ioctl(flash, FLASH_IOC_SCANBB, &part_num);

	flash_close(flash);
L1:
	return ret;
}

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

	while ((ch = getopt(argc, argv, "p:a:l:h")) != -1) {
		switch (ch) {
		case 'a':
			if (flag || (flash_str_to_val(optarg, &start, &start_unit) < 0)) {
				printf("Invalid argument: \"%s\"\n", optarg);
				usage();
				return -EINVAL;
			}

			flag = 1;

			break;

		case 'l':
			if (flag == 2 || flash_str_to_val(optarg, (__u32 *)&size, &size_unit) < 0) {
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

	flash = get_current_flash();
	if (NULL == flash) {
		return -ENODEV;
	}

	// -a xxxblock or -a xxxpage
	if (start_unit == 'b') {
		start *= flash->erase_size;
	} else if (start_unit == 'p') {
		start *= flash->write_size;
	}

	// -l xxxblock or -l xxxpage
	if (size_unit == 'b') {
		size *= flash->erase_size;
	} else if (size_unit == 'p') {
		size *= flash->write_size;
	}

	if (size == 0)
		size = flash->write_size + flash->oob_size;

	ALIGN_UP(size, flash->write_size + flash->oob_size);

	if (start) {
		if (start + size >= flash->chip_size) {
			printf("Address 0x%08x overflow!\n", start);
			flash_close(flash);
			return -EINVAL;
		}
	} else {
		start = flash->bdev.bdev_base;
	}

	flash_close(flash);

	buff = (__u8 *)malloc(size);
	if (NULL == buff) {
		return -ENOMEM;
	}

	start &= ~(flash->write_size - 1);

	ret = flash_ioctl(flash, FLASH_IOCS_OOB_MODE, (void *)FLASH_OOB_RAW);
	// if ret < 0
	ret = flash_read(flash, buff, start, size);

	if (ret < 0) {
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

	flash = get_current_flash();
	assert(flash);

	// if option is "-p" erase the whole partiton or default current partition with no option
	if (arg_flag == 2) {
		start = flash->bdev.bdev_base;
		size  = flash->bdev.bdev_size;
	} else {
		// -a xxxblock or -a xxxpage
		if (start_unit == 'b') {
			start *= flash->erase_size;
		}
		else if (start_unit == 'p') {
			start *= flash->write_size;
		}

		// -l xxxblock or -l xxxpage
		if (size_unit == 'b') {
			size *= flash->erase_size;
		} else if (size_unit == 'p') {
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

int main(int argc, char *argv[])
{
	int i;

	struct command cmd[] = {
		{
			.name = "info",
			.main = info
		},
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
