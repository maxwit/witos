#include <drive.h>

int block_device_init();
int disk_drive_init();
int foo_drv_init();

int main(int argc, char *argv[])
{
	const char *fn;

	if (2 == argc)
		fn = argv[1];
	else
		fn = "hd.img";

	block_device_init();
	disk_drive_init();
	foo_drv_init(fn);

	return 0;
}
