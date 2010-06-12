#include <g-bios.h>
#include <flash/flash.h>
#include <flash/part.h>
#include <getopt.h>
#include <flash/part.h>


static void FlashEraseUsage(void)
{
	printf("Usage: flasherase [options <value>] [flags]\n" );
   	printf("\noptions:\n"
#if 0
		"  -p   \t\terase current partition \n"
#endif
		"  -a   \t\tset erase unit to byte and specify starting address\n"
		"  -b   \t\tset erase unit to block and specify starting block number\n"
		"  -d   \t\tset allow bad block\n"
		"  -m   \t\tset flash clean mark\n"
		"  -l   \t\tset length,default is 0\n"
#if 0
		"       \t\t-p, -a,  -b conflict with other\n"
#else
		"		\t\t-a,  -b conflict with other\n"
#endif
		 );

    printf("\nexamples:\n"
#if 0
	"  flasherase -p\n"
#endif
	"  flasherase -a 1M -l 32K\n"
	"  flasherase -b 100 -l 16 -d \n"
    	  );
}


// fixme: bad logic!
int main(int argc, char *argv[])
{
	struct flash_chip *flash	   = NULL;
	struct partition *pCurPart = NULL;
	u32 nRet 		= 0;
	u32 nAddress  	= 0;
	u32 nEraseStart  	= 0;
	u32 nLen			= 0;
	u32 nEraseLen;
	u32 flags  	= 0;
	int c				= 0;
	u8 bFlag;
	char *optarg;


	if (1 == argc)
	{
		FlashEraseUsage();
		return -EINVAL;
	}

	pCurPart = part_open(PART_CURR, OP_RDWR);
	BUG_ON(NULL == pCurPart);

	flash = pCurPart->host;
	BUG_ON(NULL == flash);

	nEraseLen = flash->erase_size;

	part_close(pCurPart);


	bFlag = 0;

	while ((c = getopt(argc, argv, "b:a:dml:p", &optarg)) != -1)
	{
		switch (c)
		{
		case 'b':
			if (string2value(optarg, &nAddress) < 0 || bFlag)
			{
				FlashEraseUsage();
				return -EINVAL;
			}

			bFlag = 1;

			nEraseStart = nAddress << flash->erase_shift;

			break;

		case 'a':
			if (string2value(optarg, &nAddress) < 0 || bFlag)
			{
				FlashEraseUsage();
				return -EINVAL;
			}

			bFlag = 1;

			nEraseStart = nAddress;

			break;

		case 'l':
			if (string2value(optarg, &nLen) < 0)
			{
				FlashEraseUsage();
				return -EINVAL;
			}
			nEraseLen = nLen;
			break;

		case 'p':
			nEraseStart = pCurPart->attr->part_base;
			nEraseLen   = pCurPart->attr->part_size;

			break;

		case 'm':
			flags |= EDF_JFFS2;
			break;

		case 'd':
			flags |= EDF_ALLOWBB;
			break;

		case '?':
		case ':':
		case 'h':
		default:
			FlashEraseUsage();
			return -EINVAL;

		}
	}

	if (flash->chip_size < nEraseStart + nEraseLen)
	{
		printf("Out of chip size!\n");
		return -EINVAL;
	}

	//aligned:
	ALIGN_UP(nEraseStart, flash->write_size);
	ALIGN_UP(nEraseLen, flash->block_size);

	printf("[0x%08x : 0x%08x]\n", nEraseStart, nEraseLen);
	nRet = flash_erase(flash, nEraseStart, nEraseLen, flags);

	flash_close(flash);

	return nRet;
}



