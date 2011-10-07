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

static void free_skb_list(struct list_node *qu)
{
	struct list_node *first;
	struct sock_buff *skb;

	while (!list_is_empty(qu))
	{
		first = qu->next;

		list_del_node(first);
		skb = container_of(first, struct sock_buff, node);
		skb_free(skb);
	}
}

// fixme
void tcp_send_packet(struct sock_buff * skb);

int pseudo_calculate_checksum(struct sock_buff *skb, u16 *checksum);

struct sock_buff *tcp_recv_packet(struct socket *sock);

// fixme!
static __u32 seq_num = 1, ack_num;

int sk_close(int fd)
{
	struct socket *sock;
	struct sock_buff *skb;
	struct tcp_header *tcp_hdr;
	int opt_len;

	sock = get_sock(fd);
	if (NULL == sock)
		return -EINVAL;

	// TODO:  add code here

	opt_len = 0;
	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + TCP_HDR_LEN + opt_len, 0);

	skb->sock = sock;

	skb->data -= TCP_HDR_LEN + opt_len;
	skb->size += TCP_HDR_LEN + opt_len;

	tcp_hdr = (struct tcp_header *)skb->data;
	//
	tcp_hdr->src_port = sock->saddr[SA_SRC].sin_port;
	tcp_hdr->dst_port = sock->saddr[SA_DST].sin_port;
	tcp_hdr->seq_num  = CPU_TO_BE32(seq_num);
	tcp_hdr->ack_num  = CPU_TO_BE32(ack_num);
	tcp_hdr->hdr_len  = (TCP_HDR_LEN + opt_len) / 4;
	tcp_hdr->reserve1 = 0;
	tcp_hdr->reserve2 = 0;
	tcp_hdr->flg_urg  = 0;
	tcp_hdr->flg_ack  = 1;
	tcp_hdr->flg_psh  = 0;
	tcp_hdr->flg_rst  = 0;
	tcp_hdr->flg_syn  = 0;
	tcp_hdr->flg_fin  = 1;
	tcp_hdr->win_size = 0; // CPU_TO_BE16(32792);
	tcp_hdr->checksum = 0;
	tcp_hdr->urg_ptr  = 0;

	if (opt_len > 0)
		memset(tcp_hdr->options, 0x0, opt_len);

	pseudo_calculate_checksum(skb, &tcp_hdr->checksum);

	ip_send_packet(skb, PROT_TCP);

	while (1)
	{
		skb = tcp_recv_packet(sock);
		if (!skb)
		{
			printf("tcp_recv_packet() failed!\n");
			return -ETIMEDOUT; // fixme
		}

		tcp_hdr = (struct tcp_header *)skb->data;
		if (tcp_hdr->flg_ack && tcp_hdr->flg_fin)
			break;
	}

	seq_num++;
	ack_num = BE32_TO_CPU(tcp_hdr->seq_num) + 1;

	skb_free(skb);

	// ACK
	opt_len = 0;
	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + TCP_HDR_LEN + opt_len, 0);
	// if null

	skb->sock = sock;

	skb->data -= TCP_HDR_LEN + opt_len;
	skb->size += TCP_HDR_LEN + opt_len;

	tcp_hdr = (struct tcp_header *)skb->data;
	//
	tcp_hdr->src_port = sock->saddr[SA_SRC].sin_port;
	tcp_hdr->dst_port = sock->saddr[SA_DST].sin_port;
	tcp_hdr->seq_num  = CPU_TO_BE32(seq_num);
	tcp_hdr->ack_num  = CPU_TO_BE32(ack_num);
	tcp_hdr->hdr_len  = (TCP_HDR_LEN + opt_len) / 4;
	tcp_hdr->reserve1 = 0;
	tcp_hdr->reserve2 = 0;
	tcp_hdr->flg_urg  = 0;
	tcp_hdr->flg_ack  = 1;
	tcp_hdr->flg_psh  = 0;
	tcp_hdr->flg_rst  = 0;
	tcp_hdr->flg_syn  = 0;
	tcp_hdr->flg_fin  = 0;
	tcp_hdr->win_size = CPU_TO_BE16(32832);
	tcp_hdr->checksum = 0;
	tcp_hdr->urg_ptr  = 0;

	if (opt_len > 0)
		memset(tcp_hdr->options, 0x0, opt_len);

	pseudo_calculate_checksum(skb, &tcp_hdr->checksum);

	ip_send_packet(skb, PROT_TCP);

	free_skb_list(&sock->rx_qu);
	free_skb_list(&sock->tx_qu);
	free(sock);

	g_sock_fds[fd] = NULL;

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
	int ret = 0;
	struct socket *sock;
	const struct sockaddr_in *sa;
	struct sockaddr_in *sin;

	sock = get_sock(fd);
	if (NULL == sock)
	{
		// printf
		return -EINVAL;
	}

	sin = &sock->saddr[SA_SRC];

	sin->sin_family = addr->sa_family;

	switch (addr->sa_family)
	{
	case AF_INET:
	default: // fixme: move default down
		sa = (const struct sockaddr_in *)addr;

		sin->sin_port = sa->sin_port ? sa->sin_port : htons(55555); // fixme: NetPortAlloc

		if (sa->sin_addr.s_addr == htonl(INADDR_ANY))
			ret = ndev_ioctl(NULL, NIOC_GET_IP, &sin->sin_addr.s_addr);
		else
			sin->sin_addr = sa->sin_addr;

		break;
	}

	return ret;
}

int connect(int fd, const struct sockaddr *addr, socklen_t len)
{
	struct socket *sock;
	struct sock_buff *skb;
	struct tcp_header *tcp_hdr;
	int opt_len;

	sock = get_sock(fd);

	if (NULL == sock)
	{
		// DPRINT
		return -ENOENT;
	}

	memcpy(&sock->saddr[SA_DST], addr, len);

	// SYN
	opt_len = 20;
	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + TCP_HDR_LEN + opt_len, 0);
	// if null

	skb->sock = sock;

	skb->data -= TCP_HDR_LEN + opt_len;
	skb->size += TCP_HDR_LEN + opt_len;

	tcp_hdr = (struct tcp_header *)skb->data;
	//
	tcp_hdr->src_port = sock->saddr[SA_SRC].sin_port;
	tcp_hdr->dst_port = sock->saddr[SA_DST].sin_port;
	tcp_hdr->seq_num  = CPU_TO_BE32(seq_num);
	tcp_hdr->ack_num  = 0;
	tcp_hdr->hdr_len  = (TCP_HDR_LEN + opt_len) / 4;
	tcp_hdr->reserve1 = 0;
	tcp_hdr->reserve2 = 0;
	tcp_hdr->flg_urg  = 0;
	tcp_hdr->flg_ack  = 0;
	tcp_hdr->flg_psh  = 0;
	tcp_hdr->flg_rst  = 0;
	tcp_hdr->flg_syn  = 1;
	tcp_hdr->flg_fin  = 0;
	tcp_hdr->win_size = 0; // CPU_TO_BE16(32792);
	tcp_hdr->checksum = 0;
	tcp_hdr->urg_ptr  = 0;

	if (opt_len > 0)
		memset(tcp_hdr->options, 0x0, opt_len);

	pseudo_calculate_checksum(skb, &tcp_hdr->checksum);

	ip_send_packet(skb, PROT_TCP);

	// SYN | ACK
	skb = tcp_recv_packet(sock);

	if (!skb)
	{
		printf("tcp_recv_packet() failed!\n");
		return -ETIMEDOUT; // fixme
	}

	tcp_hdr = (struct tcp_header *)skb->data;

	DPRINT("%s() 0x%08x: src_port = 0x%x, dst_port = 0x%x, flags = 0x%08x\n",
		__func__, skb->data, tcp_hdr->src_port, tcp_hdr->dst_port, *(&tcp_hdr->ack_num + 1));

	if (!tcp_hdr->flg_ack || !tcp_hdr->flg_syn)
		return -EIO;

	seq_num++;
	ack_num = BE32_TO_CPU(tcp_hdr->seq_num) + 1;

	skb_free(skb);

	// ACK
	opt_len = 12;
	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + TCP_HDR_LEN + opt_len, 0);
	// if null

	skb->sock = sock;

	skb->data -= TCP_HDR_LEN + opt_len;
	skb->size += TCP_HDR_LEN + opt_len;

	tcp_hdr = (struct tcp_header *)skb->data;
	//
	tcp_hdr->src_port = sock->saddr[SA_SRC].sin_port;
	tcp_hdr->dst_port = sock->saddr[SA_DST].sin_port;
	tcp_hdr->seq_num  = CPU_TO_BE32(seq_num);
	tcp_hdr->ack_num  = CPU_TO_BE32(ack_num);
	tcp_hdr->hdr_len  = (TCP_HDR_LEN + opt_len) / 4;
	tcp_hdr->reserve1 = 0;
	tcp_hdr->reserve2 = 0;
	tcp_hdr->flg_urg  = 0;
	tcp_hdr->flg_ack  = 1;
	tcp_hdr->flg_psh  = 0;
	tcp_hdr->flg_rst  = 0;
	tcp_hdr->flg_syn  = 0;
	tcp_hdr->flg_fin  = 0;
	tcp_hdr->win_size = CPU_TO_BE16(32832);
	tcp_hdr->checksum = 0;
	tcp_hdr->urg_ptr  = 0;

	if (opt_len > 0)
		memset(tcp_hdr->options, 0x0, opt_len);

	pseudo_calculate_checksum(skb, &tcp_hdr->checksum);

	ip_send_packet(skb, PROT_TCP);

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
	int opt_len;
	struct tcp_header *tcp_hdr;

	sock = get_sock(fd);

	if (NULL == sock)
	{
		// printf
		return -EINVAL;
	}

	opt_len = 0;
	skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN + TCP_HDR_LEN + opt_len, n);
	// if null

	memcpy(skb->data, buf, n);
	skb->sock = sock;

	// TCP xmit
	skb->data -= TCP_HDR_LEN + opt_len;
	skb->size += TCP_HDR_LEN + opt_len;

	tcp_hdr = (struct tcp_header *)skb->data;
	//
	tcp_hdr->src_port = sock->saddr[SA_SRC].sin_port;
	tcp_hdr->dst_port = sock->saddr[SA_DST].sin_port;
	tcp_hdr->seq_num  = CPU_TO_BE32(seq_num);
	tcp_hdr->ack_num  = CPU_TO_BE32(ack_num);
	tcp_hdr->hdr_len  = (TCP_HDR_LEN + opt_len) / 4;
	tcp_hdr->reserve1 = 0;
	tcp_hdr->reserve2 = 0;
	tcp_hdr->flg_urg  = 0;
	tcp_hdr->flg_ack  = 1;
	tcp_hdr->flg_psh  = 1;
	tcp_hdr->flg_rst  = 0;
	tcp_hdr->flg_syn  = 0;
	tcp_hdr->flg_fin  = 0;
	tcp_hdr->win_size = CPU_TO_BE16(32832);
	tcp_hdr->checksum = 0;
	tcp_hdr->urg_ptr  = 0;

	if (opt_len > 0)
		memset(tcp_hdr->options, 0x0, opt_len);

	pseudo_calculate_checksum(skb, &tcp_hdr->checksum);

	ip_send_packet(skb, PROT_TCP);

	seq_num += n;

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

static inline struct socket *get_sock(int fd)
{
	if (fd <= 0 || fd >= MAX_SOCK_NUM)
		return NULL;

	return g_sock_fds[fd];
}

struct socket *tcp_search_socket(const struct tcp_header *tcp_pkt, const struct ip_header *ip_pkt)
{
	int fd;
	struct socket *sock;
	struct sockaddr_in *saddr;
	// u32 src_ip;

	// ndev_ioctl(NULL, NIOC_GET_IP, &src_ip);

	for (fd = 1; fd < MAX_SOCK_NUM; fd++)
	{
		sock = g_sock_fds[fd];

		if (NULL == sock)
			continue;

		saddr = &sock->saddr[SA_SRC];

		if (sock->type != SOCK_STREAM)
			continue;

#if 0
		if (memcmp(saddr->sin_addr, ip_pkt->src_ip, 4) != 0)
		{
			printf("sock = 0x%x, des ip is %d (NOT %d.%d.%d.%d, %02x.%02x.%02x.%02x.%02x.%02x)\n",
				saddr,
				ip_pkt->src_ip[3],
				saddr->des_ip[0],
				saddr->des_ip[1],
				saddr->des_ip[2],
				saddr->des_ip[3],
				saddr->des_mac[0],
				saddr->des_mac[1],
				saddr->des_mac[2],
				saddr->des_mac[3],
				saddr->des_mac[4],
				saddr->des_mac[5]
				);
			continue;
		}
#endif

		if (saddr->sin_port != tcp_pkt->dst_port)
		{
#if 0
			printf("from %d, src port: 0x%x != 0x%x\n", ip_pkt->src_ip[3], BE16_TO_CPU(saddr->src_port), BE16_TO_CPU(tcp_pkt->dst_port));
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

#if 0
		memcmp(&sock->saddr[SA_DST].sin_addr, ip_hdr->src_ip, IPV4_ADR_LEN);
		memcmp(sock->saddr[SA_DST].sin_port = tcp_hdr->src_port);
#endif

		return sock;
	}

	return NULL;
}

struct socket *udp_search_socket(const struct udp_header *udp_pkt, const struct ip_header *ip_pkt)
{
	int fd;
	struct socket *sock;
	struct sockaddr_in *saddr;
	u32 src_ip;

	ndev_ioctl(NULL, NIOC_GET_IP, &src_ip);

	for (fd = 1; fd < MAX_SOCK_NUM; fd++)
	{
		sock = g_sock_fds[fd];

		if (NULL == sock)
			continue;

		saddr = &sock->saddr[SA_SRC];

		if (sock->type != SOCK_DGRAM)
			continue;

#if 0
		if (memcmp(saddr->sin_addr, ip_pkt->src_ip, 4) != 0)
		{
			printf("sock = 0x%x, des ip is %d (NOT %d.%d.%d.%d, %02x.%02x.%02x.%02x.%02x.%02x)\n",
				saddr,
				ip_pkt->src_ip[3],
				saddr->des_ip[0],
				saddr->des_ip[1],
				saddr->des_ip[2],
				saddr->des_ip[3],
				saddr->des_mac[0],
				saddr->des_mac[1],
				saddr->des_mac[2],
				saddr->des_mac[3],
				saddr->des_mac[4],
				saddr->des_mac[5]
				);
			continue;
		}
#endif

		if (saddr->sin_port != udp_pkt->dst_port)
		{
#if 0
			printf("from %d, src port: 0x%x != 0x%x\n", ip_pkt->src_ip[3], BE16_TO_CPU(saddr->src_port), BE16_TO_CPU(udp_pkt->dst_port));
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

	for (fd = 0; fd < MAX_SOCK_NUM; fd++)
		g_sock_fds[fd] = NULL;
}
