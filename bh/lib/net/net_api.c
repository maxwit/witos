#include <g-bios.h>
#include <net/net.h>

struct net_device *net_open(const char *name)
{
	return NULL;
}

int net_close(struct net_device *ndev)
{
	return 0;
}

int net_ioctl(struct net_device *ndev, int cmd, void *arg)
{
	return 0;
}

