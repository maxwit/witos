#include <drive.h>

int block_device_init();
int disk_drive_init();
int foo_card_register();

int main()
{

	block_device_init();
	disk_drive_init();
	foo_card_register();

	return 0;
}
