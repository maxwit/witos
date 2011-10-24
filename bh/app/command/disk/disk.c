#include <mmc/mmc.h>
#include <mmc/mmc_ops.h>
#include <getopt.h>
#include <string.h>
#include <app.h>

#define BUFF_LEN		512
#define BLK_LEN			512

static int scan(int argc, char *argv[])
{
	int	ret;
	struct mmc_cid *cid;
	struct mmc_host *host;

	if (argc > 1)
	{
		usage();

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

static int dump(int argc, char *argv[])
{
	int opt;
	__u8 buf[BLK_LEN];
	int ret = 0;
	__u32 addr = 0;
	struct mmc_host * host;

	while ((opt = getopt(argc, argv, "a::h")) != -1)
	{
		switch (opt)
		{
		case 'a':
			if (optarg != NULL)
			{
				ret = str_to_val(optarg, &addr);
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
			usage();
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
		printf("%08x", ((__u32*)buf)[i]);

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
	int i;

	struct command cmd[] =
	{
		{
			.name = "scan",
			.main = scan
		},
		{
			.name = "dump",
			.main = dump
		}
	};

	if (argc >= 2)
	{
		for (i = 0; i < ARRAY_ELEM_NUM(cmd); i++)
		{
			if (strcmp(argv[1], cmd[i].name) == 0)
			{
				cmd[i].main(argc - 1, argv + 1);
				return 0;
			}
		}
	}

	usage();

	return -1;
 }
