#include <g-bios.h>
#include <sysconf.h>
#include <net/net.h>
#include <net/mii.h>
#include <uart/uart.h>

struct host_addr
{
	struct sockaddr sk_addr;
	struct list_node node;
};


static struct list_node g_host_list;
static struct list_node g_ndev_list;
static struct net_device *g_curr_ndev; // fixme
#ifdef CONFIG_DEBUG
static const char *g_arp_desc[] = {"N/A", "Request", "Reply"};
#endif


static void ether_send_packet(struct sock_buff *skb, const u8 mac[], u16 eth_type);

struct socket *search_socket(const struct udp_header *, const struct ip_header *);


static void __INLINE__ mac_fill_bcast(u8 mac[])
{
	memset(mac,(u8)0xff, MAC_ADR_LEN);
}


#ifdef CONFIG_DEBUG
static void ether_info(struct ether_header *eth_head)
{

	printf("\tEther frame type: 0x%04x\n", BE16_TO_CPU(eth_head->frame_type));

	printf("\tdest mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		eth_head->des_mac[0],
		eth_head->des_mac[1],
		eth_head->des_mac[2],
		eth_head->des_mac[3],
		eth_head->des_mac[4],
		eth_head->des_mac[5]);

	printf("\tsource mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
		eth_head->src_mac[0],
		eth_head->src_mac[1],
		eth_head->src_mac[2],
		eth_head->src_mac[3],
		eth_head->src_mac[4],
		eth_head->src_mac[5]);
}


static void dump_sock_buff(const struct sock_buff *skb)
{
	int idx;
	u8 *data;

	for (idx = 0, data = skb->head; idx < skb->size; idx++, data++)
	{
		printf("%02x", *data);
	}

	printf("\n");
}
#endif

//----------------- UDP Layer -----------------
void udp_send_packet(struct sock_buff *skb)
{
	struct udp_header *udp_hdr;

	skb->data -= UDP_HDR_LEN;
	skb->size += UDP_HDR_LEN;

	udp_hdr = (struct udp_header *)skb->data;
	//
	udp_hdr->src_port = skb->sock->addr.src_port;
	udp_hdr->des_port = skb->sock->addr.des_port;
	udp_hdr->udp_len  = CPU_TO_BE16(skb->size);
	udp_hdr->chksum  = 0;

	ip_send_packet(skb, PROT_UDP);
}


struct sock_buff *udp_recv_packet(struct socket *sock)
{
	struct list_node *first;
	struct sock_buff *skb;
	u32 psr;

	while (1)
	{
		int ret;
		char key;

		ret = uart_read(CONFIG_DBGU_ID, (u8 *)&key, 1, WAIT_ASYNC);
		if (ret > 0 && key == CHAR_CTRL_C)
			return NULL;

		netif_rx_poll();

		lock_irq_psr(psr);
		if (!list_is_empty(&sock->rx_qu))
		{
			unlock_irq_psr(psr);
			break;
		}
		unlock_irq_psr(psr);

		udelay(10);
	}

	lock_irq_psr(psr);
	first = sock->rx_qu.next;
	list_del_node(first);
	unlock_irq_psr(psr);

	skb = OFF2BASE(first, struct sock_buff, node);

	return skb;
}


static int udp_layer_deliver(struct sock_buff *skb, const struct ip_header *ip_hdr)
{
	struct udp_header *udp_hdr;
	struct socket *sock;

	udp_hdr = (struct udp_header *)skb->data;

	skb->data += UDP_HDR_LEN;
	skb->size -= UDP_HDR_LEN;

	sock = search_socket(udp_hdr, ip_hdr);

	if (NULL == sock)
	{
		u8 *ip;

		ip = (u8 *)ip_hdr->src_ip;
		skb_free(skb);

		return -ENODEV;
	}

	sock->addr.des_port = udp_hdr->src_port;

	list_add_tail(&skb->node, &sock->rx_qu);

	return 0;
}


static int init_ping_packet(struct ping_packet *ping_pkt,
				const u8 *buff, u8 type, u32 size, u16 id, u16 seq)
{
	char *data = NULL;
	u32 hdr_len = sizeof(struct ping_packet);

	if (NULL == ping_pkt || NULL == buff)
	    return -EINVAL;

	ping_pkt->type    = type;
	ping_pkt->code    = 0;
	ping_pkt->chksum = 0;
	ping_pkt->id     = id;
	ping_pkt->seqno   = seq;

	data = (char *)ping_pkt + hdr_len;

	memcpy(data, buff, size - hdr_len);

	ping_pkt->chksum = ~net_calc_checksum(ping_pkt, size);

	return 0;
}


int ping_send(struct sock_buff *rx_skb, const struct ip_header *ip_hdr, u8 type)
{
	u8   ping_buff[PING_PACKET_LENGTH];
	u32 hdr_len = sizeof(struct ping_packet);
	struct socket sock;
	struct sock_buff *tx_skb;
	struct ether_header *eth_head;
	struct ping_packet *ping_pkt, *rx_ping_head;

	eth_head = (struct ether_header *)(rx_skb->data - IP_HDR_LEN - ETH_HDR_LEN);

	rx_ping_head = (struct ping_packet *)rx_skb->data;

	ping_pkt = (struct ping_packet *)ping_buff;

	tx_skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN, PING_PACKET_LENGTH);
	if (NULL == tx_skb)
	{
	    printf("%s(): skb_alloc return null\n", __FUNCTION__);
	    return -EBUSY;
	}

	memcpy(sock.addr.des_ip, ip_hdr->src_ip, IPV4_ADR_LEN);

	memcpy(sock.addr.des_mac, eth_head->src_mac, MAC_ADR_LEN);

	tx_skb->sock = &sock;

	init_ping_packet(ping_pkt, (u8 *)rx_ping_head + hdr_len, type, PING_PACKET_LENGTH, rx_ping_head->id, rx_ping_head->seqno);

	memcpy(tx_skb->data, ping_pkt, PING_PACKET_LENGTH);

	ip_send_packet(tx_skb, PROT_ICMP);

	return 0;
}


int ping_recv(struct sock_buff *rx_skb, const struct ip_header *ip_hdr)
{
	struct ping_packet *ping_hdr;

	ping_hdr = (struct ping_packet *)rx_skb->data;

	printf("%d bytes from %d.%d.%d.%d: icmp_seq=%d ttl=%d time=12.7 ms\n", // fixme :-)
	        rx_skb->size,
	        ip_hdr->src_ip[0],
	        ip_hdr->src_ip[1],
	        ip_hdr->src_ip[2],
	        ip_hdr->src_ip[3],
	        BE16_TO_CPU(ping_hdr->seqno),
	        ip_hdr->ttl);

	return 0;
}


int ping_request(struct socket *socket)
{
	int i, j;
	u8  ping_buff[PING_PACKET_LENGTH];
	u32 hdr_len = sizeof(struct ping_packet);
	struct sock_buff *skb;
	struct ping_packet *ping_pkt;


	for (i = 0; i < PING_MAX_TIMES; i++)
	{
	    skb = skb_alloc(ETH_HDR_LEN + IP_HDR_LEN, PING_PACKET_LENGTH);
	    if (NULL == skb)
	    {
	    	printf("%s(): skb_alloc failed\n", __FUNCTION__);
			return -ENOMEM;
	    }

	    skb->sock = socket;

	    ping_pkt = (struct ping_packet *)ping_buff;

	    init_ping_packet(ping_pkt, ping_buff + hdr_len,
			ICMP_TYPE_ECHO_REQUEST, PING_PACKET_LENGTH, CPU_TO_BE16(PING_DEFUALT_PID), CPU_TO_BE16(i + 1));

	    memcpy(skb->data, ping_pkt, PING_PACKET_LENGTH);

		ip_send_packet(skb, PROT_ICMP);

		for (j = 0; j < 10; j++)
		{
			mdelay(10);
			netif_rx_poll();
		}
	}

	return 0;
}


static int icmp_deliver(struct sock_buff *skb, const struct ip_header *ip_hdr)
{
	struct ping_packet *ping_hdr;
	struct ether_header *eth_head;

	ping_hdr = (struct ping_packet *)skb->data;
	eth_head = (struct ether_header *)(skb->data - IP_HDR_LEN - ETH_HDR_LEN);

	switch(ping_hdr->type)
	{
	case ICMP_TYPE_ECHO_REQUEST:
	    ping_send(skb, ip_hdr, ICMP_TYPE_ECHO_REPLY);

	    break;

	case ICMP_TYPE_ECHO_REPLY:
	    ping_recv(skb, ip_hdr);

	    break;

	case ICMP_TYPE_DEST_UNREACHABLE:
	    printf("From %d.%d.%d.%d: icmp_seq=%d Destination Host Unreachable\n",
	            ip_hdr->des_ip[0],
	            ip_hdr->des_ip[1],
	            ip_hdr->des_ip[2],
	            ip_hdr->des_ip[3],
	            BE16_TO_CPU(ping_hdr->seqno));

	    break;

	default:
	    break;
	}

	return 0;
}


int ip_layer_deliver(struct sock_buff *skb)
{
	struct ip_header *ip_hdr;

	ip_hdr = (struct ip_header *)skb->data;

	skb->data += IP_HDR_LEN;
	skb->size = BE16_TO_CPU(ip_hdr->total_len) - IP_HDR_LEN;

	if (IP_HDR_LEN != ((ip_hdr->ver_len & 0xf) << 2))
	{
		printf("Warning: ip_hdr head len not match\n");
		return -1;
	}


	switch(ip_hdr->up_prot)
	{
	case PROT_UDP:
		// printf("\tUDP received!\n");
		udp_layer_deliver(skb, ip_hdr);
		break;

	case PROT_ICMP:
		// printf("\n\tICMP received!\n");
		icmp_deliver(skb, ip_hdr);
		break;

	case PROT_TCP:
		// printf("\tTCP received!\n");
		skb_free(skb);
		break;

	case PROT_IGMP:
		printf("\tIGMP received!\n");
		skb_free(skb);
		break;

	case PROT_OSPF:
		printf("\tOSPF received!\n");
		skb_free(skb);
		break;

	default:
		printf("\n\tUnknown IP Protocol!\n");
		skb_free(skb);
		return 0;
	}


	return ip_hdr->up_prot;
}


void ip_send_packet(struct sock_buff *skb, u8 bProtocal)
{
	struct ip_header *ip_hdr;
	u32 src_ip;


	skb->data -= IP_HDR_LEN;
	skb->size += IP_HDR_LEN;

	ip_hdr = (struct ip_header *)skb->data;

	ip_hdr->ver_len  = 0x45;
	ip_hdr->tos        = 0;
	ip_hdr->total_len   = (u16)CPU_TO_BE16((u16)skb->size);
	ip_hdr->id         = (u16)CPU_TO_BE16(0); //
	ip_hdr->flag_frag   = (u16)CPU_TO_BE16(0x4000);
	ip_hdr->ttl = 0xff;
	ip_hdr->up_prot     = bProtocal;
	ip_hdr->chksum     = 0;

	net_get_ip(NULL, &src_ip);
	memcpy(ip_hdr->src_ip, &src_ip, IPV4_ADR_LEN);

	memcpy(&ip_hdr->des_ip, skb->sock->addr.des_ip, IPV4_ADR_LEN);

	ip_hdr->chksum = ~net_calc_checksum(ip_hdr, IP_HDR_LEN);

	ether_send_packet(skb, skb->sock->addr.des_mac, ETH_TYPE_IP);
}


//------------------------ ARP Layer -------------------------
void arp_send_packet(const u8 nip[], const u8 *mac, u16 op_code)
{
	struct sock_buff *skb;
	struct arp_packet *arp_pkt;
	u8 mac_adr[MAC_ADR_LEN];
	u32 src_ip;


	skb = skb_alloc(ETH_HDR_LEN, ARP_PKT_LEN);
	if (NULL == skb)
	{
		printf("%s: fail to alloc skb!\n", __FUNCTION__);
		return;
	}

	arp_pkt = (struct arp_packet *)skb->data;

	arp_pkt->hard_type = CPU_TO_BE16(1);
	arp_pkt->prot_type = ETH_TYPE_IP;
	arp_pkt->hard_size = MAC_ADR_LEN;
	arp_pkt->prot_size = IPV4_ADR_LEN;
	arp_pkt->op_code = op_code;

	net_get_mac(NULL, mac_adr); //fixme for failure
	memcpy(arp_pkt->src_mac, mac_adr, MAC_ADR_LEN);

	net_get_ip(NULL, &src_ip);
	memcpy(arp_pkt->src_ip, &src_ip, IPV4_ADR_LEN);

	if (NULL == mac)
		mac_fill_bcast(arp_pkt->des_mac);
	else
		memcpy(arp_pkt->des_mac, mac, MAC_ADR_LEN);
	memcpy(arp_pkt->des_ip, nip, IPV4_ADR_LEN);

	ether_send_packet(skb, arp_pkt->des_mac, ETH_TYPE_ARP);
}


static int arp_recv_packet(struct sock_buff *skb)
{
	struct arp_packet *arp_pkt;
	u32 ip;

	arp_pkt = (struct arp_packet *)skb->data;

	if (arp_pkt->prot_type != ETH_TYPE_IP)
	{
		printf("\tProt Error!\n");
		return -1;
	}

	memcpy(&ip, arp_pkt->src_ip, 4);

#ifdef CONFIG_DEBUG
	printf("\t%s ARP received from: %d.%d.%d.%d\n",
			g_arp_desc[BE16_TO_CPU(arp_pkt->op_code)],
			arp_pkt->src_ip[0],
			arp_pkt->src_ip[1],
			arp_pkt->src_ip[2],
			arp_pkt->src_ip[3]
			);
#endif

	switch(arp_pkt->op_code)
	{
	case ARP_OP_REP:
		if (getaddr(ip) == NULL)
		{
			struct host_addr *host;

			host = malloc(sizeof(struct host_addr));
			if (NULL == host)
				return -ENOMEM;

			memcpy(host->sk_addr.des_ip, arp_pkt->src_ip, 4);
			memcpy(host->sk_addr.des_mac, arp_pkt->src_mac, 6);
#if 0
			printf("ARP %d.%d.%d.%d<-->%02x.%02x.%02x.%02x.%02x.%02x\n",
				   host->sk_addr.des_ip[0],
				   host->sk_addr.des_ip[1],
				   host->sk_addr.des_ip[2],
				   host->sk_addr.des_ip[3],
				   host->sk_addr.des_mac[0],
				   host->sk_addr.des_mac[1],
				   host->sk_addr.des_mac[2],
				   host->sk_addr.des_mac[3],
				   host->sk_addr.des_mac[4],
				   host->sk_addr.des_mac[5]
				   );
#endif

			list_add_tail(&host->node, &g_host_list);
		}

		break;

	case ARP_OP_REQ:
		arp_send_packet(arp_pkt->src_ip, arp_pkt->src_mac, ARP_OP_REP);
		break;

	default:
		printf("\t%s(): op_code error!\n", __FUNCTION__);
		break;
	}

	skb_free(skb);

	return 0;
}


//-----------------------------------------------
int netif_rx(struct sock_buff *skb)
{
	struct ether_header *eth_head;

	eth_head = (struct ether_header *)skb->data;

	skb->data += ETH_HDR_LEN;
	skb->size -= ETH_HDR_LEN;

	switch(eth_head->frame_type)
	{
	case ETH_TYPE_ARP:
		// printf("\tARP received.\n");
		arp_recv_packet(skb);
		break;

	case ETH_TYPE_RARP:
		// printf("\tRARP packet received.\n");
		break;

	case ETH_TYPE_IP:
		// printf("\tIP packet received.\n");
		ip_layer_deliver(skb);
		break;

	default:
		// printf("\tframe type error (= 0x%04x)!\n", eth_head->frame_type);
		// skb_free(skb);
		break;
	}

	return 0;
}

//------------------ Send Package to Hardware -----------------
void ether_send_packet(struct sock_buff *skb, const u8 mac[], u16 eth_type)
{
	struct ether_header *eth_head;
	u8 mac_adr[MAC_ADR_LEN];


	skb->data -= ETH_HDR_LEN;
	skb->size += ETH_HDR_LEN;

	if (skb->data != skb->head)
	{
		printf("skb len error!\n");
		return;
	}

	eth_head = (struct ether_header *)skb->data;

	memcpy(eth_head->des_mac, mac, MAC_ADR_LEN);

	net_get_mac(NULL, mac_adr); //fixme for failure
	memcpy(eth_head->src_mac, mac_adr, MAC_ADR_LEN);

	eth_head->frame_type = eth_type;

	g_curr_ndev->send_packet(g_curr_ndev, skb);

	skb_free(skb);
}


u16 net_calc_checksum(const void *buff, u32 size)
{
	u32 n;
	u32	chksum;
	const u16 *p;

	chksum = 0;

	for (n = size, p = buff; n > 0; n -= 2, p++)
		chksum += *p;

	chksum = (chksum & 0xffff) + (chksum >> 16);
	chksum = (chksum & 0xffff) + (chksum >> 16);

	return (chksum & 0xffff);
}


struct sock_buff *skb_alloc(u32 prot_len, u32 data_len)
{
	struct sock_buff *skb;

	skb = malloc(sizeof(struct sock_buff));
	if (NULL == skb)
		return NULL;

	skb->head = malloc(prot_len + data_len);
	if (NULL == skb->head)
	{
		printf("%s(): malloc failed (size = %d bytes)!\n", __FUNCTION__, prot_len + data_len);
		return NULL;
	}

	skb->data = skb->head + prot_len;
	//skb-->pSkbTail = skb->data + data_len;
	skb->size  = data_len;

	list_head_init(&skb->node);

	return skb;
}

void skb_free(struct sock_buff *skb)
{
	BUG_ON(!skb);

	free(skb->head);
	free(skb);
}


struct sockaddr *getaddr(u32 nip)
{
	struct list_node *iter;
	struct host_addr *host;
	u32 ulPsr;
	u32 *dip;
	struct sockaddr *addr = NULL;

	lock_irq_psr(ulPsr);

	list_for_each(iter, &g_host_list)
	{
		host = OFF2BASE(iter, struct host_addr, node);

		dip = (u32 *)host->sk_addr.des_ip;
		if (*dip == nip)
		{
			addr = &host->sk_addr;
			break;
		}
	}

	unlock_irq_psr(ulPsr);

	return addr;
}


int net_get_ip(struct net_device *ndev, u32 *ip)
{
	struct net_config *net_cfg;

	sysconf_get_net_info(&net_cfg);

	if (NULL == net_cfg)
	{
		BUG();
		return -EIO;
	}

	*ip = net_cfg->local_ip;

	return 0;
}


int net_set_ip(struct net_device *ndev, u32 ip)
{
	struct net_config *net_cfg;

	sysconf_get_net_info(&net_cfg);

	if (NULL == net_cfg)
	{
		BUG();
		return -EIO;
	}

	net_cfg->local_ip = ip;

	return 0;
}


int net_get_server_ip(u32 *ip)
{
	struct net_config *net_cfg;

	sysconf_get_net_info(&net_cfg);

	if (NULL == net_cfg)
	{
		BUG();
		return -EIO;
	}

	*ip = net_cfg->server_ip;

	return 0;
}


int net_set_server_ip(u32 ip)
{
	struct net_config *net_cfg;

	sysconf_get_net_info(&net_cfg);

	if (NULL == net_cfg)
	{
		BUG();
		return -EIO;
	}

	net_cfg->server_ip = ip;

	return 0;
}


int net_set_mask(struct net_device *ndev, u32 mask)
{
	struct net_config *net_cfg;

	sysconf_get_net_info(&net_cfg);

	if (NULL == net_cfg)
	{
		BUG();
		return -EIO;
	}

	net_cfg->net_mask = mask;

	return 0;
}

int net_dev_register(struct net_device *ndev)
{
	int i;
	// int speed;
	struct mii_phy *phy;

	if (NULL == ndev || NULL == ndev->send_packet)
		return -EINVAL;

	list_add_tail(&ndev->ndev_node, &g_ndev_list);
	g_curr_ndev = ndev;

	// fixme
	if (!ndev->phy_mask || !ndev->mdio_read || !ndev->mdio_write)
		return 0;

	// detecting PHY
	for (i = 0; i < 32; i++)
	{
		if (!((1 << i) & ndev->phy_mask))
			continue;

		phy = mii_phy_probe(ndev, i);

		if (phy)
		{
			phy->ndev = ndev;
			list_add_tail(&phy->phy_node, &ndev->phy_list);

			mii_reset_phy(ndev, phy);

			// fixme
			printf("PHY found @ MII[%d]: ID1 = 0x%04x, ID2 = 0x%04x\n",
				i, phy->ven_id, phy->dev_id);

#if 0
			printf("Detecting ethernet speed ... ");
			speed = mii_get_link_status(phy);

			if (speed < 0)
			{
				printf("NOT linked, please check the cable!\n");
			}
			else
			{
				switch(speed)
				{
				case ETHER_SPEED_10M_HD:
					printf("10M Half Duplex activated!\n");
					break;

				case ETHER_SPEED_10M_FD:
					printf("10M Full Duplex activated!\n");
					break;

				case ETHER_SPEED_100M_HD:
					printf("100M Half Duplex activated!\n");
					break;

				case ETHER_SPEED_100M_FD:
					printf("100M Full Duplex activated!\n");
					break;

				default:
					printf("Unknown speed (%d)!\n", speed);
					break;
				}
			}
#endif
			// break;
		}
	}

	return 0;
}

int net_get_mac(struct net_device *ndev, u8 pMacAddr[])
{
	memcpy(pMacAddr, g_curr_ndev->mac_adr, MAC_ADR_LEN);
	return 0;
}

int net_set_mac(struct net_device *ndev, const u8 mac_adr[])
{
	if (NULL == g_curr_ndev)
		return -ENODEV;

	memcpy(g_curr_ndev->mac_adr, mac_adr, MAC_ADR_LEN);

	if (g_curr_ndev->set_mac_adr)
	{
		g_curr_ndev->set_mac_adr(g_curr_ndev, mac_adr);
	}

	return 0;
}

struct net_device *net_dev_new(u32 chip_size)
{
	static int ndev_count = 0; // fixme!!
	struct net_device *ndev;
	u32 core_size = (sizeof(struct net_device) + WORD_SIZE - 1) & ~(WORD_SIZE - 1);

	ndev = zalloc(core_size + chip_size);
	if (NULL == ndev)
		return NULL;

	ndev->chip = (struct net_device *)((u8 *)ndev + core_size);
	ndev->phy_mask = 0xFFFFFFFF;

	// set default name
	sprintf(ndev->ifx_name, "eth%d", ndev_count);
	ndev_count++;

	list_head_init(&ndev->ndev_node);
	list_head_init(&ndev->phy_list);

	return ndev;
}


#ifndef CONFIG_IRQ_SUPPORT
int netif_rx_poll()
{
	if (!g_curr_ndev)
		return -ENODEV;

	if (!g_curr_ndev->ndev_poll)
		return 0;

	return g_curr_ndev->ndev_poll(g_curr_ndev);
}
#endif


int net_check_link_status()
{
	int speed, phy_count = 0;
	struct net_device *ndev;
	struct mii_phy *phy;
	struct list_node *ndev_ln, *phy_ln;

	list_for_each(ndev_ln, &g_ndev_list)
	{
		ndev = OFF2BASE(ndev_ln, struct net_device, ndev_node);

		list_for_each(phy_ln, &ndev->phy_list)
		{
			phy = OFF2BASE(phy_ln, struct mii_phy, phy_node);

			printf("Detecting ethernet speed for %s(%s) PHY%d ... ",
				ndev->ifx_name, ndev->chip_name, phy->mii_id);

			speed = mii_get_link_status(phy);

			if (speed < 0)
			{
				printf("NOT linked, please check the cable!\n");
			}
			else
			{
				switch(speed)
				{
				case ETHER_SPEED_10M_HD:
					printf("10M Half Duplex activated!\n");
					break;

				case ETHER_SPEED_10M_FD:
					printf("10M Full Duplex activated!\n");
					break;

				case ETHER_SPEED_100M_HD:
					printf("100M Half Duplex activated!\n");
					break;

				case ETHER_SPEED_100M_FD:
					printf("100M Full Duplex activated!\n");
					break;

				default:
					printf("Unknown speed (%d)!\n", speed);
					break;
				}
			}

			phy_count++;
		}
	}

	if (0 == phy_count)
		printf("No PHY found!\n");

	return phy_count;
}


void socket_init(void);

static int __INIT__ net_core_init(void)
{
	socket_init();

	list_head_init(&g_host_list);
	list_head_init(&g_ndev_list);

	g_curr_ndev = NULL;

	return 0;
}

SUBSYS_INIT(net_core_init);

