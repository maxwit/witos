#include <sysconf.h>
#include <net/net.h>

#define PING_LEN 8
#define MAX_LEN 512

int main(int argc, char *argv[])
{
	int ret, i;
	int fd;
	__u32 nip;
	__u16 seq_num = 1;
	__u8 buf[MAX_LEN];
	__u8 ip_hdr_len;
	__u8 ping_buff[PING_LEN];
	char dest_ip[IPV4_STR_LEN];
	struct ping_packet *ping_pkt;
	struct ip_header *ip;
	struct sockaddr_in local_addr, dest_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);

	if (1 == argc) // use default server as ping server if no argument supplied.
	{
		net_get_server_ip(&nip);
		ip_to_str(dest_ip, nip);
	}
	else
	{
		ret = str_to_ip((__u8 *)&nip, argv[1]);
		if (ret < 0)
		{
			printf("Illegal IP (%s)!\n", argv[1]);
			return ret;
		}

		strcpy(dest_ip, argv[1]);
	}

	fd = socket(AF_INET, SOCK_RAW, 0);
	printf("PING %s:\n", dest_ip);

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = nip;

	for (i = 0; i < PING_MAX_TIMES; i++)
	{
	    ping_pkt = (struct ping_packet *)ping_buff;

		ping_pkt->type   = ICMP_TYPE_ECHO_REQUEST;
		ping_pkt->code   = 0;
		ping_pkt->chksum = 0;
		ping_pkt->id     = 0;
		ping_pkt->seqno  = htons(seq_num++);
		ping_pkt->chksum = ~net_calc_checksum(ping_pkt, sizeof(ping_buff));

		sendto(fd, ping_buff, sizeof(ping_buff), 0,
				(struct sockaddr *)&dest_addr, addr_len);

		ret = recvfrom(fd, buf, MAX_LEN, 0,
				(struct sockaddr *)&dest_addr, &addr_len);
		if (ret < 0)
		{
			printf("recvfrom error\n");
			return ret;
		}

		ip = (struct ip_header *)buf;
		ip_hdr_len = (ip->ver_len & 0xf) << 2;
		ping_pkt= (struct ping_packet *)(buf + ip_hdr_len);

		printf("%d bytes from %d.%d.%d.%d: icmp_req = %d, ttl = %d\n",
				ntohs(ip->total_len), ip->src_ip[0], ip->src_ip[1],
				ip->src_ip[2], ip->src_ip[3], ntohs(ping_pkt->seqno), ip->ttl);
	}

	sk_close(fd);
	return ret;
}
