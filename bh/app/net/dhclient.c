#include <getopt.h>
#include <sysconf.h>
#include <net/net.h>

// DHCP opcode
#define CLIENT_REQUEST	1
#define SERVER_ACK		2

#define ETHER_TYPE 1

// DHCP option type code
#define DHCP_MESSAGE		53
#define DHCP_MESSAGE_LEN	1
#define DHCP_REQUEST_IP		50
#define DHCP_REQUEST_IP_LEN 4

// DHCP message type value
#define DHCPDISCOVER	0x1
#define DHCPOFFER		0x2
#define DHCPREQUEST		0x3
#define DHCPDECLINE		0x4
#define DHCPACK			0x5
#define DHCPNAK			0x6
#define DHCPRELEASE		0x7
#define DHCPINFOM		0x8

struct dhcp_packet
{
	char	op;
	char	htype;
	char	hlen;
	char	hops;
	u32		xid;
	short	sec;
	short	flags;
	u32		ciaddr;
	u32		yiaddr;
	u32		siaddr;
	u32		giaddr;
	char	chaddr[16];
	char	sname[64];
	char	file[128];
	// actually, the options field's size is 64 bytes(include the magic cookie)
	char	magic_cookie[4];
	char	option[60];
}__PACKED__;

static void init_dhcp_packet(struct dhcp_packet *packet, u32 xid, u8 mac_addr[])
{
	memset(packet, 0x0, sizeof(*packet));

	packet->op		= CLIENT_REQUEST;
	packet->htype	= ETHER_TYPE;
	packet->hlen	= MAC_ADR_LEN;
	packet->hops	= 0x00;
	packet->xid     = xid;
	packet->sec     = 0x00;
	packet->flags   = 0x00;
	packet->yiaddr  = 0x00;
	memcpy(packet->chaddr, mac_addr, MAC_ADR_LEN);
	// magic cookie
	packet->magic_cookie[0] = 0x63;
	packet->magic_cookie[1] = 0x82;
	packet->magic_cookie[2] = 0x53;
	packet->magic_cookie[3] = 0x63;
}

static int send_dhcp_discover(int sockfd, struct dhcp_packet *packet, struct sockaddr_in *remote_addr)
{
	packet->option[0] = DHCP_MESSAGE;
	packet->option[1] = DHCP_MESSAGE_LEN;
	packet->option[2] = DHCPDISCOVER;
	packet->option[3] = 0xff;

	return sendto(sockfd, packet, sizeof(*packet), 0,
				(const struct sockaddr *)remote_addr, sizeof(struct sockaddr));
}

static int send_dhcp_request(int sockfd, struct dhcp_packet *packet, struct sockaddr_in *remote_addr)
{
	u32 request_ip;

	request_ip = packet->yiaddr;

	packet->option[0] = DHCP_MESSAGE;
	packet->option[1] = DHCP_MESSAGE_LEN;
	packet->option[2] = DHCPREQUEST;
	packet->option[3] = DHCP_REQUEST_IP;
	packet->option[4] = DHCP_REQUEST_IP_LEN;
	// request ip
	packet->option[5] = request_ip & 0xff;
	packet->option[6] = request_ip >> 8 & 0xff;
	packet->option[7] = request_ip >> 16 & 0xff;
	packet->option[8] = request_ip >> 24;

	packet->option[9] = 0xff;

	return sendto(sockfd, packet, sizeof(*packet), 0,
				(const struct sockaddr *)remote_addr, sizeof(*remote_addr));
}

// fixme!!!
long random(void)
{
	int ret;
	u8 mac_addr[MAC_ADR_LEN];

	ret = ndev_ioctl(NULL, NIOC_GET_MAC, mac_addr);
	// if ret < 0 ...

	// just get a random number, mac addr is a random num
	return mac_addr[4] << 24 | mac_addr[3] << 16 | mac_addr[2] << 8 | mac_addr[1];
}

static void usage(const char *app)
{
	printf("Usage:\n\t%s [-s]\n"
		"options:\n"
		"\t-s: update Server IP to DHCP server.",
		app);
}

int main(int argc, char *argv[])
{
	int ret, i;
	int sockfd;
	struct sockaddr_in remote_addr, local_addr;
	struct dhcp_packet packet;
	socklen_t addrlen;
	char ip_str[IPV4_STR_LEN];
	u8 mac_addr[MAC_ADR_LEN];
	u32 xid = random();
	int opt;
	BOOL sync_svr = FALSE;

	while ((opt = getopt(argc, argv, "sh", NULL)) != -1)
	{
		switch (opt)
		{
		case 's':
			sync_svr = TRUE;
			break;

		default:
			usage(argv[0]);
			return -EINVAL;
		}
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		printf("socket() failed!\n");
		return sockfd;
	}

	local_addr.sin_addr.s_addr = 0;
	local_addr.sin_port = CPU_TO_BE16(68);

	memset(&remote_addr.sin_addr, 0xff, IPV4_ADR_LEN);
	remote_addr.sin_port = CPU_TO_BE16(67);

	ret = bind(sockfd, (const struct sockaddr *) &local_addr, sizeof(local_addr));
	if (ret < 0)
	{
		printf("connect() failed!\n");
		goto ERROR;
	}

	ret = ndev_ioctl(NULL, NIOC_GET_MAC, mac_addr);

	init_dhcp_packet(&packet, xid, mac_addr);

	ret = send_dhcp_discover(sockfd, &packet, &remote_addr);
	if (ret < 0)
	{
		printf("send dhcp discover packet failed!");
		goto ERROR;
	}

	addrlen = sizeof(remote_addr);

	for (i = 0; i < 3; i++)
	{
		ret = recvfrom(sockfd, &packet, sizeof(packet), 0,
				(struct sockaddr *)&remote_addr, &addrlen);
		if (ret <= 0)
		{
			printf("recv dhcp offer packet failed!");
			goto ERROR;
		}

		if (packet.xid == xid)
		{
			ip_to_str(ip_str, packet.yiaddr);

			if (packet.option[2] != DHCPOFFER)
			{
				printf("Can't recv offer packet!\n");
				ret = -ENONET;
				goto ERROR;
			}
			break;
		}
	}

	if (i >= 3)
	{
		printf("Timeout!\n");
		ret = -ENONET;
		goto ERROR;
	}

	remote_addr.sin_addr.s_addr = 0xFFFFFFFF;

	ret = send_dhcp_request(sockfd, &packet, &remote_addr);
	if (ret < 0)
	{
		printf("send dhcp request packet failed!");
		goto ERROR;
	}

	for (i = 0; i < 3; i++)
	{
		ret = recvfrom(sockfd, &packet, sizeof(packet), 0,
				(struct sockaddr *)&remote_addr, &addrlen);

		if (ret <= 0)
		{
			printf("recv dhcp ACK/NAK packet failed!");
			goto ERROR;
		}

		if (packet.xid == xid)
		{
			ip_to_str(ip_str, packet.yiaddr);

			if (packet.option[2] != DHCPACK)
			{
				printf("Can't recv ACK packet!\n");
				ret = -ENONET;
				goto ERROR;
			}

			break;
		}
	}

	if (i >= 3)
	{
		printf("Tomeout\n");
		ret = -ENONET;
		goto ERROR;
	}

	ndev_ioctl(NULL, NIOC_SET_IP, (void *)packet.yiaddr);

	if (sync_svr)
	{
		net_set_server_ip(remote_addr.sin_addr.s_addr);
		sysconf_save();
	}

	ip_to_str(ip_str, remote_addr.sin_addr.s_addr);
	printf("server ip: %s\n", ip_str);
	ip_to_str(ip_str, packet.yiaddr);
	printf("local  ip: %s\n", ip_str);

	close(sockfd);

	return 0;

ERROR:
	close(sockfd);

	return ret;
}
