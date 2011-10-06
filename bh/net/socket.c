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
		printf("%s: fail to alloc socket!\n", __func__);
		return -ENOMEM;
	}

	sock->type = type;
	memset(sock->saddr, 0, sizeof(sock->saddr));
	list_head_init(&sock->tx_qu);
	list_head_init(&sock->rx_qu);

	g_sock_fds[fd] = sock;

	return fd;
}

// fix for full destroy
int sk_close(int fd)
{
	if (fd < 1 || fd >= MAX_SOCK_NUM || NULL == g_sock_fds[fd])
	{
		printf("%s: invalid fd!\n", __func__);
		return -EINVAL;
	}

	SAFE_FREE(g_sock_fds[fd]);

	return 0;
}

struct eth_addr *gethostaddr(const u32 nip)
{
	u32 count = 0;
	struct eth_addr *remote_addr;

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

		remote_addr = getaddr(nip);
		if (remote_addr)
			break;

		// TODO: add re-send code here
		udelay(3);
		count++;
	}

	return remote_addr;
}

int bind(int fd, const struct sockaddr *addr, socklen_t len)
{
	struct socket *sock;

	sock = get_sock(fd);

	if (NULL == sock)
	{
		// printf
		return -EINVAL;
	}

	memcpy(&sock->saddr[SA_SRC], addr, len);

	return 0;
}

// fixme
void tcp_send_packet(struct sock_buff * skb);

int connect(int fd, const struct sockaddr *addr, socklen_t len)
{
	struct socket *sock;
	struct sock_buff *skb;
	static char buff[20] = {0x02, 0x04, 0x40, 0x0c, 0x04, 0x02, 0x08, 0x0a, 0x00, 0x95, 0x7b, 0xea, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x06};

	sock = get_sock(fd);

	if (NULL == sock)
	{
		// DPRINT
		return -ENOENT;
	}

	memcpy(&sock->saddr[SA_DST], addr, len);

	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + TCP_HDR_LEN, 20);
	// if null

	skb->sock = sock;

	memcpy(skb->data, buff, 20);

	tcp_send_packet(skb);

	return 0;
}

//
ssize_t sendto(int fd, const void *buff, u32 buff_size, int flags,
			const struct sockaddr *dest_addr, socklen_t addr_size)
{
	struct socket *sock;
	struct sock_buff *skb;

	sock = get_sock(fd);

	if (NULL == sock)
	{
		// printf
		return -EINVAL;
	}

	memcpy(&sock->saddr[SA_DST], dest_addr, addr_size);

	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN, buff_size);
	// if null

	skb->sock = sock;

	memcpy(skb->data, buff, buff_size);

	udp_send_packet(skb);

	return buff_size;
}

long recvfrom(int fd, void *buf, u32 n, int flags,
		struct sockaddr *src_addr, socklen_t *addrlen)
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

	// fixme
	// if (sizeof(skb->remote_addr) > *addrlen) return -EINVAL;
	*addrlen = sizeof(struct sockaddr_in);
	memcpy(src_addr, &sock->saddr[SA_DST], *addrlen);

	memcpy(buf, skb->data, pkt_len);

	skb_free(skb);

	return pkt_len;
}

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
	struct sockaddr_in *sk_adr;
	u32 src_ip;

	ndev_ioctl(NULL, NIOC_GET_IP, &src_ip);

	for (fd = 1; fd < MAX_SOCK_NUM; fd++)
	{
		sock = g_sock_fds[fd];

		if (NULL == sock)
			continue;

		sk_adr = &sock->saddr[SA_SRC];

#if 0
		if (memcmp(sk_adr->sin_addr, ip_pkt->src_ip, 4) != 0)
		{
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
			continue;
		}
#endif

		if (sk_adr->sin_port != udp_pkt->des_port)
		{
#if 0
			printf("from %d, src port: 0x%x != 0x%x\n", ip_pkt->src_ip[3], BE16_TO_CPU(sk_adr->src_port), BE16_TO_CPU(udp_pkt->des_port));
#endif
			continue;
		}

#if 0
		if (memcmp(&src_ip, ip_pkt->des_ip, 4) != 0)
		{
			printf("src ip: %d.%d.%d.%d\n",
				ip_pkt->des_ip[0],
				ip_pkt->des_ip[1],
				ip_pkt->des_ip[2],
				ip_pkt->des_ip[3]
				);

			continue;
		}
#endif

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
