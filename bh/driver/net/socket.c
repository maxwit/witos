#include <g-bios.h>
#include <net/net.h>
#include <uart/uart.h>


#define MAX_SOCK_NUM  32

static struct socket *g_sock_fds[MAX_SOCK_NUM];

static struct socket *get_sock(int fd);

//
int socket(int domain, int type, int protocol)
{
	int fd;
	struct socket *sock;

	for (fd = 1; fd < MAX_SOCK_NUM; fd++)
	{
		if (NULL == g_sock_fds[fd])
			goto alloc_sock;
	}

	return -ENODEV;

alloc_sock:
	sock = malloc(sizeof(struct socket));

	if (NULL == sock)
	{
		printf("%s: fail to alloc socket!\n", __FUNCTION__);
		return -ENOMEM;
	}

	memset(&sock->addr, 0, sizeof(struct sockaddr));

	list_head_init(&sock->tx_qu);
	list_head_init(&sock->rx_qu);

	g_sock_fds[fd] = sock;

	return fd;
}

// fix for full destroy
int close(int fd)
{
	if (fd < 1 || fd >= MAX_SOCK_NUM || NULL == g_sock_fds[fd])
	{
		printf("%s: invalid fd!\n", __FUNCTION__);
		return -EINVAL;
	}

	SAFE_FREE(g_sock_fds[fd]);

	return 0;
}


struct sockaddr *gethostaddr(const char *sip)
{
	u32 nip;
	u32 count = 0;
	struct sockaddr *sk_addr;

	if(str_to_ip((u8 *)&nip, sip))
	{
		printf("%s(): fail to convert ip %s\n", __FUNCTION__, sip);
		return NULL;
	}

	arp_send_packet((u8 *)&nip, NULL, ARP_OP_REQ);

	while (count < 100000)
	{
		int ret;
		u8  key;

		ret = uart_read(CONFIG_DBGU_ID, &key, 1, WAIT_ASYNC);
		if (ret > 0 && key == CHAR_CTRL_C)
		{
			return NULL;
		}

		netif_rx_poll();

		sk_addr = getaddr(nip);
		if (sk_addr)
			break;

		// TODO: add re-send code here
		udelay(3);
		count++;
	}

	return sk_addr;
}

int connect(int fd, const struct sockaddr *addr, socklen_t len)
{
	struct socket *sock;

	sock = get_sock(fd);

	if (NULL == sock)
	{
		// printf
		return -EINVAL;
	}

	memcpy(&sock->addr, addr, len);

	return 0;
}

//
long send(int fd, const void *buf, u32 n)
{
	struct socket *sock;
	struct sock_buff *skb;

	sock = get_sock(fd);

	if (NULL == sock)
	{
		// printf
		return -EINVAL;
	}

	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN, n);

	skb->sock = sock;

	memcpy(skb->data, buf, n);

	udp_send_packet(skb);

	return n;
}

//
long recv(int fd, void *buf, u32 n)
{
	struct socket *sock;
	struct sock_buff *skb;
	u32 pkt_len;

	sock = get_sock(fd);

	if (NULL == sock)
	{
		return -EINVAL;
	}

	skb = udp_recv_packet(sock);
	if(NULL == skb)
		return 0;

	// fixme !
	pkt_len   = skb->size <= n ? skb->size : n;

	memcpy(buf, skb->data, pkt_len);

	skb_free(skb);

	return pkt_len;
}


struct socket *get_sock(int fd)
{
	if (fd <= 0 || fd >= MAX_SOCK_NUM)
		return NULL;

	return g_sock_fds[fd];
}

struct socket *search_socket(const struct udp_header *udp_pkt, const struct ip_header *ip_pkt)
{
	int fd;
	struct socket *sock;
	struct sockaddr *sk_adr;
	u32 src_ip;


	net_get_ip(NULL, &src_ip);

	for (fd = 1; fd < MAX_SOCK_NUM; fd++)
	{
		sock = g_sock_fds[fd];

		if (NULL == sock)
			continue;

		sk_adr = &sock->addr;

		if (memcmp(sk_adr->des_ip, ip_pkt->src_ip, 4) != 0)
		{
#if 0
			printf("sock = 0x%x, des ip is %d (NOT %d.%d.%d.%d, %02x.%02x.%02x.%02x.%02x.%02x)\n",
				sk_adr,
				ip_pkt->src_ip[3],
				sk_adr->des_ip[0],
				sk_adr->des_ip[1],
				sk_adr->des_ip[2],
				sk_adr->des_ip[3],
				sk_adr->des_mac[0],
				sk_adr->des_mac[1],
				sk_adr->des_mac[2],
				sk_adr->des_mac[3],
				sk_adr->des_mac[4],
				sk_adr->des_mac[5]
				);
#endif
			continue;
		}

		if (sk_adr->src_port != udp_pkt->des_port)
		{
#if 0
			printf("from %d, src port: 0x%x != 0x%x\n", ip_pkt->src_ip[3], BE16_TO_CPU(sk_adr->src_port), BE16_TO_CPU(udp_pkt->des_port));
#endif
			continue;
		}

		if (memcmp(&src_ip, ip_pkt->des_ip, 4) != 0)
		{
#if 0
			printf("src ip: %d.%d.%d.%d\n",
				ip_pkt->des_ip[0],
				ip_pkt->des_ip[1],
				ip_pkt->des_ip[2],
				ip_pkt->des_ip[3]
				);
#endif
			continue;
		}

		return sock;
	}

	return NULL;
}

void socket_init(void)
{
	int fd;

	g_sock_fds[0] = NULL;//(struct socket *)g_sock_fds;

	for (fd = 1; fd < MAX_SOCK_NUM; fd++)
		g_sock_fds[fd] = NULL;

}
