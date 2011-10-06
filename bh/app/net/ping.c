#include <sysconf.h>
#include <net/net.h>

static int init_ping_packet(struct ping_packet *ping_pkt,
				const u8 *buff, u8 type, u32 size, u16 id, u16 seq)
{
	char *data = NULL;
	u32 hdr_len = sizeof(struct ping_packet);

	if (NULL == ping_pkt || NULL == buff)
	    return -EINVAL;

	ping_pkt->type   = type;
	ping_pkt->code   = 0;
	ping_pkt->chksum = 0;
	ping_pkt->id     = id;
	ping_pkt->seqno  = seq;

	data = (char *)ping_pkt + hdr_len;

	memcpy(data, buff, size - hdr_len);

	ping_pkt->chksum = ~net_calc_checksum(ping_pkt, size);

	return 0;
}

static int ping_request(struct socket *sock, u32 dst_ip)
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
			printf("%s(): skb_alloc failed\n", __func__);
			return -ENOMEM;
	    }

	    skb->sock = sock;
		memcpy(&sock->saddr[SA_DST].sin_addr, &dst_ip, IPV4_ADR_LEN);

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

int main(int argc, char *argv[])
{
	int ret;
	struct socket sock; // fixme
	char dest_ip[IPV4_STR_LEN];
	u32 nip;
	u32 src_ip;

	if (1 == argc) // use default server as ping server if no argument supplied.
	{
		struct net_config *net_cfg;

		net_cfg = sysconf_get_net_info();
		nip = net_cfg->server_ip;
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

#if 0
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
#endif

	memset(&sock, 0, sizeof(sock));

	ndev_ioctl(NULL, NIOC_GET_IP, &src_ip);
	sock.saddr[SA_SRC].sin_addr.s_addr = src_ip;

	ret = ping_request(&sock, nip);

	return ret;
}
