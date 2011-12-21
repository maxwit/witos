#pragma once

#include <types.h>
#include <list.h>
#include <device.h>

// fixme
#define	PF_INET		2	/* IP protocol family.  */
#define	AF_INET		PF_INET /*Address family*/

#define INADDR_ANY  0

enum __socket_type {
	SOCK_STREAM    = 1,
	SOCK_DGRAM     = 2,
	SOCK_RAW       = 3,
	SOCK_RDM       = 4,
	SOCK_SEQPACKET = 5,
	SOCK_DCCP      = 6,
	SOCK_PACKET    = 10,
	SOCK_CLOEXEC   = 02000000,
	SOCK_NONBLOCK  = 04000
};

/* Bits in the FLAGS argument to `send', `recv', et al.  */
enum __flags_type {
	MSG_OOB          = 0x01,
	MSG_PEEK         = 0x02,
	MSG_DONTROUTE    = 0x04,
	MSG_TRYHARD      = MSG_DONTROUTE,
	MSG_CTRUNC       = 0x08,
	MSG_PROXY        = 0x10,
	MSG_TRUNC        = 0x20,
	MSG_DONTWAIT     = 0x40,
	MSG_EOR          = 0x80,
	MSG_WAITALL      = 0x100,
	MSG_FIN          = 0x200,
	MSG_SYN          = 0x400,
	MSG_CONFIRM      = 0x800,
	MSG_RST          = 0x1000,
	MSG_ERRQUEUE     = 0x2000,
	MSG_NOSIGNAL     = 0x4000,
	MSG_MORE         = 0x8000,
	MSG_WAITFORONE   = 0x10000,
	MSG_CMSG_CLOEXEC = 0x40000000
};

#define PROT_ICMP 1
#define PROT_IGMP 2
#define PROT_TCP  6
#define PROT_UDP  17
#define PROT_OSPF 89
#define PROT_ETH	10 //fixme

#define ETH_TYPE_IP   CPU_TO_BE16(0x0800)
#define ETH_TYPE_ARP  CPU_TO_BE16(0x0806)
#define ETH_TYPE_RARP CPU_TO_BE16(0x0835)

#define IPV4_STR_LEN    16
#define IPV4_ADR_LEN    4
#define MAC_ADR_LEN     6
#define MAC_STR_LEN	    18

#define ETH_HDR_LEN     14
#define ARP_PKT_LEN     28
#define IP_HDR_LEN      20

#define UDP_HDR_LEN     8
#define MAX_ETH_LEN     (1500 + ETH_HDR_LEN)
#define MIN_ETH_LEN     46

#define TCP_HDR_LEN     20

// ping
#define PING_PACKET_LENGTH      64
#define PING_MAX_TIMES          3
#define PING_DEFUALT_PID        1

// std ports:
#define STD_PORT_TFTP   69

#define MKIP(a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | a)

enum EtherSpeed {
	ETHER_SPEED_UNKNOW,
	ETHER_SPEED_10M_HD,
	ETHER_SPEED_10M_FD,
	ETHER_SPEED_100M_HD,
	ETHER_SPEED_100M_FD,
	ETHER_SPEED_1000M_HD,
	ETHER_SPEED_1000M_FD,
};

struct socket;
struct sockaddr;

//
struct ether_header {
	__u8 des_mac[MAC_ADR_LEN];
	__u8 src_mac[MAC_ADR_LEN];
	__u16 frame_type;
};

//
#define ARP_OP_REQ  CPU_TO_BE16(1)
#define ARP_OP_REP  CPU_TO_BE16(2)

struct arp_packet {
	__u16  hard_type;
	__u16  prot_type;
	__u8   hard_size;
	__u8   prot_size;
	__u16  op_code;
	__u8   src_mac[MAC_ADR_LEN];
	__u8   src_ip[IPV4_ADR_LEN];
	__u8   des_mac[MAC_ADR_LEN];
	__u8   des_ip[IPV4_ADR_LEN];
} __PACKED__;

//
struct ip_header {
	__u8   ver_len;
	__u8   tos;
	__u16  total_len;
	__u16  id;
	__u16  flag_frag;
	__u8   ttl;
	__u8   up_prot;
	__u16  chksum;
	__u8   src_ip[IPV4_ADR_LEN];
	__u8   des_ip[IPV4_ADR_LEN];
};

//
#define ICMP_TYPE_ECHO_REPLY         0
#define ICMP_TYPE_DEST_UNREACHABLE   3
#define ICMP_TYPE_ECHO_REQUEST       8

//
struct icmp_packet {
	__u8  type;
	__u8  code;
	__u16 chksum;
};

//
struct ping_packet {
	__u8    type;
	__u8    code;
	__u16  chksum;
	__u16  id;
	__u16  seqno;
};

//
struct udp_header {
	__u16 src_port;
	__u16 dst_port;
	__u16 udp_len;
	__u16 checksum;
};

#define FLG_FIN  (1 << 0)
#define FLG_SYN  (1 << 1)
#define FLG_RST  (1 << 2)
#define FLG_PSH  (1 << 3)
#define FLG_ACK  (1 << 4)
#define FLG_URG  (1 << 5)

struct tcp_header {
	__u16 src_port;
	__u16 dst_port;
	__u32 seq_num;
	__u32 ack_num;
#if 0
	__u16 reserve1:4;
	__u16 hdr_len:4;
	__u16 flg_fin:1;
	__u16 flg_syn:1;
	__u16 flg_rst:1;
	__u16 flg_psh:1;
	__u16 flg_ack:1;
	__u16 flg_urg:1;
	__u16 reserve2:2;
#else
#ifdef __LITTLE_ENDIAN
	__u8 reserve: 4;
	__u8 hdr_len: 4;
#else
	__u8 hdr_len: 4;
	__u8 reserve: 4;
#endif
	__u8 flags;
#endif
	__u16 win_size;
	__u16 checksum;
	__u16 urg_ptr;
	__u8  options[0];
};

#define MAX_TCP_OPT_LEN 64

struct tcp_option {
	__u8 opt[MAX_TCP_OPT_LEN];
	__u16 len;
};
///////////////////////////////////

typedef __u32 socklen_t;
typedef unsigned short sa_family_t;

#define __SOCKADDR_COMMON(sa_prefix) \
	sa_family_t sa_prefix##family

struct sockaddr {
	__SOCKADDR_COMMON(sa_);
	char sa_data[14];
};

struct in_addr {
	__u32 s_addr;
};

struct sockaddr_in {
	__SOCKADDR_COMMON(sin_);
	unsigned short sin_port;
	struct in_addr sin_addr;

	unsigned char sin_zero[sizeof(struct sockaddr) -
				sizeof(unsigned short) -
				sizeof(unsigned short) -
				sizeof(struct in_addr)];
};

enum {
	SA_SRC,
	SA_DST
};

enum tcp_state {
	TCPS_CLOSED,
	TCPS_LISTEN,
	TCPS_SYN_SENT,
	TCPS_SYN_RCVD,
	TCPS_ESTABLISHED,
	TCPS_CLOSE_WAIT,
	TCPS_FIN_WAIT1,
	TCPS_CLOSING,
	TCPS_LAST_ACK,
	TCPS_FIN_WAIT2,
	TCPS_TIME_WAIT,
};

struct socket {
	int type;
	int protocol;
	int obstruct_flags;
	struct list_node tx_qu, rx_qu;
	struct sockaddr_in saddr[2]; // fixme: sockaddr instead
	enum tcp_state state;
	// int tmp;
	__u32 seq_num, ack_num;
};

struct eth_addr {
	__u8  ip[4];
	__u8  mac[6];
};

struct sock_buff {
	__u8  *head;
	__u8  *data;
	__u16  size;

	struct list_node node;
	struct socket *sock;
	// struct sockaddr_in remote_addr;
};

struct ndev_stat {
	__u32 rx_packets;
	__u32 tx_packets;
	__u32 tx_errors;
	__u32 rx_errors;
};

struct link_status {
	int connected;
	enum EtherSpeed link_speed;
};

//
#define NET_NAME_LEN 16

struct net_device {
	struct device *dev;
	const char *chip_name;
	char ifx_name[NET_NAME_LEN];
	struct list_node ndev_node;
	//
	__u32 ip;
	__u32 mask;
	__u8  mac_addr[MAC_ADR_LEN];
	void *chip;

	struct ndev_stat stat;

	//
	int (*send_packet)(struct net_device *ndev, struct sock_buff *skb);
	int (*set_mac_addr)(struct net_device *ndev, const __u8 mac[]);
#ifndef CONFIG_IRQ_SUPPORT
	int (*ndev_poll)(struct net_device *ndev);
#endif

	// PHY
	__u32 phy_mask;
	struct list_node phy_list;
	__u16 (*mdio_read)(struct net_device *ndev, __u8 mii_id, __u8 reg);
	void (*mdio_write)(struct net_device *ndev, __u8 mii_id, __u8 reg, __u16 val);
};

static inline __u16 htons(__u16 val)
{
	return CPU_TO_BE16(val);
}

static inline __u32 htonl(__u32 val)
{
	return CPU_TO_BE32(val);
}

static inline __u16 ntohs(__u16 val)
{
	return BE16_TO_CPU(val);
}

static inline __u32 ntohl(__u32 val)
{
	return BE32_TO_CPU(val);
}

struct sock_buff *skb_alloc(__u32 prot_len, __u32 data_len);

void skb_free(struct sock_buff * skb);

///////////
struct eth_addr *getaddr(__u32 nip);
struct eth_addr *gethostaddr(const __u32 nip);

void arp_send_packet(const __u8 nip[], const __u8 *mac, __u16 op_code);
void ip_send_packet(struct sock_buff *skb, __u8 bProtocal);
void udp_send_packet(struct sock_buff *skb);
void tcp_send_packet(struct sock_buff *skb, __u8 flags, struct tcp_option *opt);

int ip_layer_deliver(struct sock_buff *skb);

int socket(int domain, int type, int protocol);
int bind(int fd, const struct sockaddr *addr, socklen_t len);
int connect(int fd, const struct sockaddr *addr, socklen_t len);
ssize_t send(int fd, const void *buf, size_t n, int flag);
ssize_t recv(int fd, void *buf, size_t n, int flag);
ssize_t sendto(int fd, const void *buf, __u32 n, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
long recvfrom(int fd, void *buf, __u32 n, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
int sk_close(int fd);

struct net_device *net_get_dev(const char *ifx_name);

__u16 net_calc_checksum(const void *buff, __u32 size);

struct net_device *ndev_new(size_t chip_size);

int ndev_register(struct net_device *ndev);

int netif_rx(struct sock_buff *skb);

#ifndef CONFIG_IRQ_SUPPORT
int ndev_recv_poll();
#else
#define ndev_recv_poll()
#endif

// int ping_request(struct socket *socket, struct eth_addr *remote_addr);

// int ping_request(struct socket *socket);

int ndev_check_link_status();

struct list_node *net_get_device_list(void);

#define NIOC_GET_IP     1
#define NIOC_SET_IP     2
#define NIOC_GET_MASK   3
#define NIOC_SET_MASK   4
#define NIOC_GET_MAC    5
#define NIOC_SET_MAC    6
#define NIOC_GET_STATUS 7

int ndev_ioctl(struct net_device *ndev, int cmd, void *arg);

int net_get_server_ip(__u32 *ip);

int net_set_server_ip(__u32 ip);

#define SKIOCS_FLAGS 1

int socket_ioctl(int fd, int cmd, int flags);

int qu_is_empty(int fd);
struct socket *arp_search_socket(const struct arp_packet *arp_pkt);
int net_set_server_ip(__u32 ip);
