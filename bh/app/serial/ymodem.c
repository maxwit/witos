#include <g-bios.h>
#include <loader.h>
#include <uart/ymodem.h>
#include <uart/kermit.h>
#include <flash/part.h>
#include <sysconf.h>


// TODO: to support specified address
// -a
// -g
//

int main(int argc, char *argv[])
{
	int size;
	struct loader_opt ld_opt;
	struct partition *part;

	memset(&ld_opt, 0x0, sizeof(ld_opt));

	if ((part = part_open(PART_CURR, OP_RDWR)) == NULL)
	{
		return -EINVAL;
	}

	ld_opt.part = part;

	printf("%s loading, please send the file ...\n", argv[0]);

	if (strcmp(argv[0], "kermit") == 0)
	{
		size = kermit_load(&ld_opt);
	}
	else
	{
		size = ymodem_load(&ld_opt);
	}

	// if size < 0

	part_set_image(part, ld_opt.file_name, ld_opt.load_size);

	part_close(part);

	sysconf_save();

	return size;
}

INSTALL_APPLICATION(kermit, main, NULL, NULL, NULL);

