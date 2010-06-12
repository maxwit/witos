#include <g-bios.h>
#include <sysconf.h>
#include <string.h>
#include <getopt.h>
#include <flash/part.h>
#include <bar.h>


struct erase_param
{
	FLASH_HOOK_PARAM hParam;
	struct process_bar *pBar;
};


static void erase_usage(void)
{
	printf("Usage: parterase <subcommands> [options <value>] [flags]\n"
			"options:\n"
			"  -p   \t\tset partition\n"
			"  -d   \t\tignore bad block\n"
//			"  -j   \t\tJffs2 file system.\n"
		  );
}

static int erase_process(struct flash_chip *flash, FLASH_HOOK_PARAM *pParam)
{
	struct erase_param *pErInfo;

	pErInfo = OFF2BASE(pParam, struct erase_param, hParam);

	progress_bar_set_val(pErInfo->pBar, pParam->nBlockIndex);

	return 0;
}


int main(int argc, char *argv[])
{
	int ret = 0, nDevNum;
	u32 size, start;
	u32 flags = EDF_NORMAL;
	struct flash_chip *flash;
	struct partition *part = NULL;
	u32 ch;
	struct erase_param erInfo;
	char *optarg;
	BOOL need_save = TRUE, for_jffs2 = FALSE;
	FLASH_CALLBACK callback;

	while ((ch = getopt(argc, argv, "p:djsh", &optarg)) != -1)
	{
		switch (ch)
		{
		case 'p':
			if(string2value(optarg, (u32 *)&nDevNum) < 0)
			{
		    	erase_usage();
				goto L1;
			}

			part = part_open(nDevNum, OP_RDWR);
			if (NULL == part)
			{
				erase_usage();
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
			erase_usage();
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

	callback.func = erase_process;
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



