#include <g-bios.h>
#include <stdio.h>
#include <string.h>
#include <sysconf.h>
#include <net/net.h>

#define MAX_LEN        512
#define DEF_SVR_PORT   54327

int main(int argc, char *argv[])
{
	int fd, ret;
	__u32 svr_ip;
	struct sockaddr_in local_addr, remote_addr;
	char msg[MAX_LEN];

	// fixme: getopt and endian
	if (argc == 2)
		str_to_ip((__u8 *)&svr_ip, argv[1]);
	else
		ret = net_get_server_ip(&svr_ip);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	// if

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(DEF_SVR_PORT);
	remote_addr.sin_addr.s_addr = svr_ip;

	connect(fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));

	send(fd, "abc", 3, 0);
	send(fd, "123", 2, 0);
	send(fd, "ABCD", 4, 0);
	send(fd, "I*am*quiting", 5, 0);
	send(fd, "quit", 4, 0);

	ret = recv(fd, msg, MAX_LEN, 0);
	if (ret > 0)
	{
		msg[ret] = '\0';
		printf("%d bytes received: %s\n", ret, msg);
	}

	sk_close(fd);

	return ret;
}
