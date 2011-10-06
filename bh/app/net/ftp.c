#include <g-bios.h>
#include <stdio.h>
#include <sysconf.h>
#include <net/net.h>

int main(int argc, char *argv[])
{
	int fd, ret;
	__u32 svr_ip;
	struct sockaddr_in local_addr, remote_addr;

	printf("%s %s\n", __FILE__, __TIME__);
	// fixme: getopt and endian
	if (argc == 2)
		str_to_ip((__u8 *)&svr_ip, argv[1]);
	else
		ret = net_get_server_ip(&svr_ip);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	// if

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_port = CPU_TO_BE16(1234); // fixme: NetPortAlloc
	ret = ndev_ioctl(NULL, NIOC_GET_IP, &local_addr.sin_addr.s_addr);

	ret = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_port = CPU_TO_BE16(5511);
	remote_addr.sin_addr.s_addr = svr_ip;

	int m;
	for (m = 200; m < 1200; m += 200)
	{
		connect(fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
		mdelay(m);
	}

	return ret;
}
