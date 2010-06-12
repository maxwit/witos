#include <g-bios.h>
#include <flash/flash.h>
#include <flash/part.h>
#include <getopt.h>
#include <flash/part.h>

// TODO: add process bar
int main(int argc, char *argv[])
{
	int ret;
	struct flash_chip *flash;

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

	flash_ioctl(flash, FLASH_IOC_SCANBB, NULL);

	flash_close(flash);
L1:
	return ret;
}

