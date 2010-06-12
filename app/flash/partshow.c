#include <g-bios.h>
#include <sysconf.h>
#include <string.h>
#include <getopt.h>
#include <flash/part.h>
#include <bar.h>


int main(int argc, char *argv[])
{
	struct flash_chip *flash;
	u32 flash_id;

	flash_id = 0;

	while ((flash = flash_open(flash_id)) != NULL)
	{
		part_show(flash);

		flash_close(flash);

		flash_id++;
	}

	return 0;
}

