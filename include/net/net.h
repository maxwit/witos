#pragma once
#include <types.h>
#include <list.h>

struct mii_phy;

// fixme

#define	PF_INET		2	/* IP protocol family.  */

#define	AF_INET		PF_INET /*Address family*/

enum __socket_type
{
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
enum __flags_type
{
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
#define PING_PACKET_LENGTH		64
#define PING_MAX_TIMES    		3
#define PING_DEFUALT_PID   		1

// std ports:
#define STD_PORT_TFTP   69

#define MKIP(a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | a)

enum EtherSpeed
{
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
struct ether_header
{
	u8 des_mac[MAC_ADR_LEN];
	u8 src_mac[MAC_ADR_LEN];
	u16 frame_type;
};

//
#define ARP_OP_REQ  CPU_TO_BE16(1)
#define ARP_OP_REP  CPU_TO_BE16(2)

struct arp_packet
{
	u16  hard_type;
	u16  prot_type;
	u8   hard_size;
	u8   prot_size;
	u16  op_code;
	u8   src_mac[MAC_ADR_LEN];
	u8   src_ip[IPV4_ADR_LEN];
	u8   des_mac[MAC_ADR_LEN];
	u8   des_ip[IPV4_ADR_LEN];
} __PACKED__;

//
struct ip_header
{
	u8   ver_len;
	u8   tos;
	u16  total_len;
	u16  id;
	u16  flag_frag;
	u8   ttl;
	u8   up_prot;
	u16  chksum;
	u8   src_ip[IPV4_ADR_LEN];
	u8   des_ip[IPV4_ADR_LEN];
};

//
#define ICMP_TYPE_ECHO_REPLY     	0
#define ICMP_TYPE_DEST_UNREACHABLE	3
#define ICMP_TYPE_ECHO_REQUEST   	8

//
struct icmp_packet
{
	u8  type;
	u8  code;
	u16 chksum;
};

//
struct ping_packet
{
	u8    type;
	u8    code;
	u16  chksum;
	u16  id;
	u16  seqno;
};

//
struct udp_header
{
	u16 src_port;
	u16 des_port;
	u16 udp_len;
	u16 chksum;
};

struct tcp_header
{
	__u16 src_port;
	__u16 dst_port;
	__u32 seq_num;
	__u32 ack_num;
	__u16 hdr_len:4;
	__u16 resv:6;
	__u16 flg_urg:1;
	__u16 flg_ack:1;
	__u16 flg_psh:1;
	__u16 flg_rst:1;
	__u16 flg_syn:1;
	__u16 flg_fin:1;
	__u16 win_size;
	__u16 check_sum;
	__u16 urg_pt;
};

///////////////////////////////////

typedef u32 socklen_t;

#if 1

typedef unsigned short sa_family_t;

#define __SOCKADDR_COMMON(sa_prefix) \
	sa_family_t sa_prefix##family

struct sockaddr
{
	__SOCKADDR_COMMON(sa);
	char sa_data[14];
};

struct in_addr
{
	u32 s_addr;
};

struct sockaddr_in
{
	__SOCKADDR_COMMON(sin);
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

struct socket
{
	int type;
	struct list_node tx_qu, rx_qu;
	struct sockaddr_in saddr[2]; // fixme: sockaddr instead
};

struct eth_addr
{
	u8  ip[4];
	u8  mac[6];
};

#else

struct sockaddr
{
	u8  des_ip[4];
	u8  des_mac[6];

	u16 des_port;
	u16 src_port;
};

struct socket
{
	struct list_node tx_qu, rx_qu;
	struct sockaddr addr;
};

#endif

struct sock_buff
{
	u8  *head;
	u8  *data;
	u16  size;

	struct list_node node;
	struct socket *sock;
	// struct sockaddr_in remote_addr;
};

struct net_device_stat
{
	u32 rx_packets;
	u32 tx_packets;
	u32 tx_errors;
	u32 rx_errors;
};

struct link_status
{
	int connected;
	enum EtherSpeed link_speed;
};

//
#define NET_NAME_LEN 16

struct net_device
{
	const char *chip_name;
	char ifx_name[NET_NAME_LEN];
	struct list_node ndev_node;
	//
	u32 ip;
	u32 mask;
	u8  mac_addr[MAC_ADR_LEN];
	void *chip;

	struct net_device_stat stat;

	//
	int (*send_packet)(struct net_device *ndev, struct sock_buff *skb);
	int (*set_mac_addr)(struct net_device *ndev, const u8 mac[]);
#ifndef CONFIG_IRQ_SUPPORT
	int (*ndev_poll)(struct net_device *ndev);
#endif

	// PHY
	u32 phy_mask;
	struct list_node phy_list;
	u16 (*mdio_read)(struct net_device *ndev, u8 mii_id, u8 reg);
	void (*mdio_write)(struct net_device *ndev, u8 mii_id, u8 reg, u16 val);
};

struct sock_buff *skb_alloc(u32 prot_len, u32 data_len);

void skb_free(struct sock_buff * skb);

///////////
struct eth_addr *gethostaddr(const u32 nip);

void udp_send_packet(struct sock_buff *skb);

struct sock_buff *udp_recv_packet(struct socket *sock);

void ip_send_packet(struct sock_buff *skb, u8 bProtocal);

int ip_layer_deliver(struct sock_buff *skb);

void arp_send_packet(const u8 nip[], const u8 *mac, u16 op_code);

int socket(int domain, int type, int protocol);

struct eth_addr *getaddr(u32 nip);

int bind(int fd, const struct sockaddr *addr, socklen_t len);

int connect(int fd, const struct sockaddr *addr, socklen_t len);

long send(int fd, const void *buf, u32 n);

ssize_t sendto(int fd, const void *buf, u32 n, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

long recv(int fd, void *buf, u32 n);

long recvfrom(int fd, void *buf, u32 n, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

int sk_close(int fd);

struct net_device *net_get_dev(const char *ifx_name);

u16 net_calc_checksum(const void *buff, u32 size);

struct net_device *net_dev_new(u32 ulChipSize);

int net_dev_register(struct net_device *ndev);

int netif_rx(struct sock_buff *skb);

#ifndef CONFIG_IRQ_SUPPORT
int netif_rx_poll();
#else
#define netif_rx_poll()
#endif

// int ping_request(struct socket *socket, struct eth_addr *remote_addr);

// int ping_request(struct socket *socket);

int net_check_link_status();

struct list_node *net_get_device_list(void);

#define NIOC_GET_IP     1
#define NIOC_SET_IP     2
#define NIOC_GET_MASK   3
#define NIOC_SET_MASK   4
#define NIOC_GET_MAC    5
#define NIOC_SET_MAC    6
#define NIOC_GET_STATUS 7

int ndev_ioctl(struct net_device *ndev, int cmd, void *arg);
