#include <mmc/mmc.h>
#include <mmc/mmc_ops.h>
#include <getopt.h>
#include <string.h>
#include <app.h>

#define BUFF_LEN		512
#define BLK_LEN			512

static void mmc_usage(int argc, char *argv[])
{
#if 0

Usage: disk <command> [<args>]
command list:
    scan     scan all MMC/SD devices, echo informations
    dump     read and echo MMC/SD datas in each block
    write    write data from memory to disk drive

See 'disk <command> -h' for more information on a specific command.

generic options:
    -a <address>   start disk address
    -m <memory>    start memory address
	-s <size>      data size (1/2/4 bytes)
	-h             this help

#endif
}

static void mmc_scan_usage(void)
{
	printf("\nUsage: mmc scan\n");
}

static int scan(int argc, char *argv[])
{
	int	ret;
	struct mmc_cid *cid;
	struct mmc_host *host;

	if (argc > 1)
	{
		mmc_scan_usage();

		return -EINVAL;
	}

	host = mmc_get_host(0);
	if(host == NULL)
	{
		printf("Error: Can't Get MMC Host!\n");
		return -ENODEV;
	}

	ret = mmc_sd_detect_card(host);
	if (ret < 0)
	{
		printf("Error: No SD Found!\n");
		return -ENODEV;
	}

	cid = (struct mmc_cid* )(host->card.raw_cid);

	printf("Manufacturer ID: %x\n"
			"OEM/Application ID: %x\n"
			"Product Name: %c%c%c%c%c\n"
			"Product serial number: %x\n",
			cid->mid, cid->oid,cid->pnm[4],
			cid->pnm[3],cid->pnm[2],cid->pnm[1],cid->pnm[0],
			cid->psn);

	return 0;
}

static void mmc_dump_usage(void)
{
	printf("\nUsage: mmc dump [option <value>]\n"
			"\nOptions:\n"
			"  -a \tset start address\n"
			"  -h \tthis help\n"
			);
}

static int dump(int argc, char *argv[])
{
	int opt;
	u8 buf[BLK_LEN];
	int ret = 0;
	u32 addr = 0;
	struct mmc_host * host;

	while ((opt = getopt(argc, argv, "a::h")) != -1)
	{
		switch (opt)
		{
		case 'a':
			if (optarg != NULL)
			{
				ret = string2value(optarg, &addr);
				if (ret < 0)
				{
					printf("Error: Address error!\n");

					return -EINVAL;
				}
			}

			printf("Dump from the address: 0x%x\n", addr);
			break;

		case 'h':
		default:
			mmc_dump_usage();
			return 0;
		}
	}

	host = mmc_get_host(0);
	if (!host)
	{
		printf("Error: No MMC Host Found");
		return -ENODEV;
	}

	if (!host->card.card_name[0])
	{
		printf("Error: No MMC Card Found");
		return -ENODEV;
	}

	memset(buf, 0xbb, BLK_LEN);

	ret = mmc_read_blk(host, buf, addr);
	if (ret < 0)
	{
		printf("Error: Fail to Read!\n");
		return ret;
	}

	int i;

	for (i = 0; i < BLK_LEN / 4; i++)
	{
		printf("%08x", ((u32*)buf)[i]);

		if ((i + 1)% 8)
		{
			printf(" ");
		}
		else
		{
			printf("\n");
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int i = 0;

	struct cmd_info command[] =
	{
		{
			.name = "scan",
			.cmd  = scan
		},
		{
			.name = "dump",
			.cmd   = dump
		}
	};

	if (argc >= 2)
	{
		for (i = 0; i < ARRAY_ELEM_NUM(command); i++)
		{
			if (strcmp(argv[1], command[i].name) == 0)
			{
				command[i].cmd(argc - 1, argv + 1);
				return 0;
			}
		}
	}

	mmc_usage(argc, argv);

	return -1;
 }
