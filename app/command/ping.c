#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/net.h>
#include <unistd.h>

#define PING_LEN 8
#define MAX_LEN 512

int main(int argc, char *argv[])
{
	int ret, i;
	int fd;
	int ch;
	__u32 nip;
	int ping_times = PING_MAX_TIMES;
	int lost_packet_count = 0;
	__u16 seq_num = 1;
	__u8 buf[MAX_LEN];
	__u8 ip_hdr_len;
	__u8 ping_buff[PING_LEN];
	char dest_ip[IPV4_STR_LEN];
	struct ping_packet *ping_pkt;
	struct ip_header *ip;
	struct sockaddr_in local_addr, dest_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);

	while ((ch = getopt(argc, argv, "c:")) != -1) {
		switch(ch) {
		case 'c':
			if (str_to_val(optarg, (unsigned long *)&ping_times) < 0) {
				printf("Invalid argument: \"%s\"\n", optarg);
				return -EINVAL;
			}
			break;

		default:
			usage();
			return -EINVAL;
		}
	}

	if (optind < argc - 1)
		return -EINVAL;

	if (optind == argc - 1) {
		ret = str_to_ip((__u8 *)&nip, argv[optind]);
		if (ret < 0) {
			printf("Illegal IP (%s)!\n", argv[optind]);
			return ret;
		}

		strcpy(dest_ip, argv[optind]);
	} else {
		net_get_server_ip(&nip);
		ip_to_str(dest_ip, nip);// use default server as ping server if no argument supplied.
	}

	fd = socket(AF_INET, SOCK_RAW, PROT_ICMP);

	ret = socket_ioctl(fd, SKIOCS_FLAGS, 1);
	if (ret < 0) {
		printf("Socket is not found\n");
		return ret;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ret = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = nip;

	printf("PING %s:\n", dest_ip);

	for (i = 0; i < ping_times; i++) {
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
		if (ret <= 0) {
			lost_packet_count++;
		} else {
			ip = (struct ip_header *)buf;
			ip_hdr_len = (ip->ver_len & 0xf) << 2;
			ping_pkt= (struct ping_packet *)(buf + ip_hdr_len);

			printf("%d bytes from %d.%d.%d.%d: icmp_req = %d, ttl = %d\n",
					ntohs(ip->total_len),
					ip->src_ip[0], ip->src_ip[1],
					ip->src_ip[2], ip->src_ip[3],
					ntohs(ping_pkt->seqno), ip->ttl);
		}
	}

	printf("---------%s ping statistics--------\n", dest_ip);

	printf("%d packet transmitted, %d received, %d%% packet loss, time 27ms\n",
			ping_times, ping_times - lost_packet_count,
			lost_packet_count  * 100 / ping_times);

	sk_close(fd);
	return ret;
}
