#include <net/net.h>
#include <g-bios.h>
#include <sysconf.h>
#include <flash/flash.h>


int main(int argc, char *argv[])
{
	int ret;

	ret = sysconf_reset();

	if (ret < 0)
	{
		printf("fail to reset system config (ret = %d)!\n", ret);
		return ret;
	}

	ret = sysconf_save();

	return ret;
}
