#include <g-bios.h>
#include <sysconf.h>
#include <net/net.h>


int main(int argc, char *argv[])
{
	int ret;
	struct sockaddr *sk_addr;
	struct socket sock; // fixme
	char dest_ip[IPV4_STR_LEN];
	u32 nip;


	if (1 == argc) // use default server as ping server if no argument supplied.
	{
		struct net_config *pNetConf;

		sysconf_get_net_info(&pNetConf);
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

	sk_addr = getaddr(nip);

	if (NULL == sk_addr)
	{
		sk_addr = gethostaddr(dest_ip);

		if (NULL == sk_addr)
		{
			printf("Fail to find host %s!\n",dest_ip);
			return -EIO;
		}
	}

	memcpy(&sock.addr, sk_addr, sizeof(struct sockaddr));

    ret = ping_request(&sock);

	return ret;
}



