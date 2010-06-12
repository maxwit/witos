#pragma once
#include <types.h>
#include <list.h>


struct mii_phy;

// fixme
#define AF_INET     0

#define SOCK_DGRAM  0
#define SOCK_STREAM 0

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

#define ETH_HDR_LEN     14
#define ARP_PKT_LEN     28
#define IP_HDR_LEN      20

#define UDP_HDR_LEN     8
#define MAX_ETH_LEN     (1500 + ETH_HDR_LEN)
#define MIN_ETH_LEN     46

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

struct sock_buff
{
	u8  *head;
	u8  *data;
	u16  size;

	struct list_node node;
	struct socket *sock;
};


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

//
#define NET_NAME_LEN 16

struct net_device
{
	const char *chip_name;
	char ifx_name[NET_NAME_LEN];
	struct list_node ndev_node;
	u8 mac_adr[MAC_ADR_LEN];
	void *chip;
	//
	int (*send_packet)(struct net_device *ndev, struct sock_buff *skb);
	int (*set_mac_adr)(struct net_device *ndev, const u8 mac[]);
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
struct sockaddr *gethostaddr(const char *sip);


void udp_send_packet(struct sock_buff *skb);

struct sock_buff *udp_recv_packet(struct socket *sock);

void ip_send_packet(struct sock_buff *skb, u8 bProtocal);

int ip_layer_deliver(struct sock_buff *skb);

void arp_send_packet(const u8 nip[], const u8 *mac, u16 op_code);

///////////////////////////////////

typedef u32 socklen_t;

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


int socket(int domain, int type, int protocol);

struct sockaddr *getaddr(u32 ip);

int connect(int fd, const struct sockaddr *addr, socklen_t len);

long send(int fd, const void *buf, u32 n);

long recv(int fd, void *buf, u32 n);

int close(int fd);


#define NFS_PATH_LEN   256

struct net_config
{
	u32  local_ip;
	u32  net_mask;
	u32  server_ip;
	u8   mac_adr[MAC_ADR_LEN];
};


int net_get_ip(struct net_device *ndev, u32 *ip);

int net_set_ip(struct net_device *ndev, u32 ip);

int net_get_mask(struct net_device *ndev, u32 *mask);

int net_set_mask(struct net_device *ndev, u32 mask);

int net_get_server_ip(u32 *ip);

int net_set_server_ip(u32 ip);

int net_get_mac(struct net_device *ndev, u8 vbMAC[MAC_ADR_LEN]);

int net_set_mac(struct net_device *ndev, const u8 vbMAC[MAC_ADR_LEN]);

u16 net_calc_checksum(const void *buff, u32 size);

struct net_device *net_dev_new(u32 ulChipSize);

int net_dev_register(struct net_device *ndev);

int netif_rx(struct sock_buff *skb);

#ifndef CONFIG_IRQ_SUPPORT
int netif_rx_poll();
#else
#define netif_rx_poll()
#endif

int ping_request(struct socket *socket);

int net_check_link_status();

