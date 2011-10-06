#include <g-bios.h>
#include <stdio.h>
#include <string.h>
#include <sysconf.h>
#include <net/net.h>

#define DEF_SVR_PORT 5511

int main(int argc, char *argv[])
{
	int fd, ret;
	__u32 svr_ip;
	struct sockaddr_in local_addr, remote_addr;
	const char *msg;

	// fixme: getopt and endian
	if (argc == 2)
		str_to_ip((__u8 *)&svr_ip, argv[1]);
	else
		ret = net_get_server_ip(&svr_ip);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	// if

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_port = CPU_TO_BE16(55555); // fixme: NetPortAlloc
	ret = ndev_ioctl(NULL, NIOC_GET_IP, &local_addr.sin_addr.s_addr);

	ret = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_port = CPU_TO_BE16(DEF_SVR_PORT);
	remote_addr.sin_addr.s_addr = svr_ip;

	connect(fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));

	msg = "MaxWit g-bios";
	send(fd, msg, strlen(msg));

	sk_close(fd);

	return ret;
}
