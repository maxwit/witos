#include <sysconf.h>
#include <flash/flash.h>

int main(int argc, char *argv[])
{
	struct linux_config *linux_conf = sysconf_get_linux_param();
	struct partition *part;

	part = part_open(PART_CURR, OP_RDONLY);

	linux_conf->root_dev = part_get_index(part);

	part_close(part);

	return 0;
}
