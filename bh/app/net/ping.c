#include <sysconf.h>
#include <net/net.h>

int main(int argc, char *argv[])
{
	int ret;
	struct eth_addr *remote_addr;
	struct socket sock; // fixme
	char dest_ip[IPV4_STR_LEN];
	u32 nip;
	u32 src_ip;

	if (1 == argc) // use default server as ping server if no argument supplied.
	{
		struct net_config *pNetConf;

		pNetConf = sysconf_get_net_info();
		nip = pNetConf->server_ip;
		ip_to_str(dest_ip, nip);
	}
	else
	{
		ret = str_to_ip((u8 *)&nip, argv[1]);
		if (ret < 0)
		{
			printf("Illegal IP (%s)!\n", argv[1]);
			return ret;
		}

		strcpy(dest_ip, argv[1]);
	}

	printf("PING %s:\n", dest_ip);

	remote_addr = getaddr(nip);

	// fixme get mac addr in ip layer
	if (NULL == remote_addr)
	{
		remote_addr = gethostaddr(nip);

		if (NULL == remote_addr)
		{
			printf("Fail to find host %s!\n",dest_ip);
			return -EIO;
		}
	}

	ndev_ioctl(NULL, NIOC_GET_IP, &src_ip);
	memset(&sock, 0, sizeof(sock));
	sock.addr.sin_addr.s_addr = src_ip;

	ret = ping_request(&sock, remote_addr);

	return ret;
}
