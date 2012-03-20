#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <net/net.h>
#include <unistd.h>

#define FLAG_IP    (1 << 0)
#define FLAG_MASK  (1 << 1)
#define FLAG_MAC   (1 << 2)
#define FLAG_CFG   (1 << 3)

static int show_net_info(struct net_device *ndev)
{
	__u8 local[IPV4_ADR_LEN];
	__u8 mask[IPV4_ADR_LEN];
	__u8 mac[MAC_ADR_LEN];
	struct link_status link;
	struct ndev_stat stat;
	const char *speed;

	ndev_ioctl(ndev, NIOC_GET_IP, local);
	ndev_ioctl(ndev, NIOC_GET_MASK, mask);
	ndev_ioctl(ndev, NIOC_GET_MAC, mac);
	ndev_ioctl(ndev, NIOC_GET_LINK, &link);
	ndev_ioctl(ndev, NIOC_GET_STAT, &stat);

	if (link.connected) {
		switch (link.speed) {
		case ETHER_SPEED_10M_HD:
			speed = "10M_HD";
			break;

		case ETHER_SPEED_10M_FD:
			speed = "10M_FD";
			break;

		case ETHER_SPEED_100M_HD:
			speed = "100M_HD";
			break;

		case ETHER_SPEED_100M_FD:
			speed = "100M_FD";
			break;

		case ETHER_SPEED_1000M_HD:
			speed = "1000M_HD";
			break;

		case ETHER_SPEED_1000M_FD:
			speed = "1000M_FD";
			break;

		default:
			speed = "Unknown";
			break;
		}
	} else {
		speed = "Unknown";
	}

	printf("%-8schip name: \"%s\"\n"
		"\tlocal addr: %d.%d.%d.%d\n"
		"\tlocal mask: %d.%d.%d.%d\n"
		"\tMAC addr: %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\tconnected:%s speed:%s\n"
		"\tRX packets:%d errors:%d\n"
		"\tTX packets:%d errors:%d\n",
		ndev->ifx_name,
		ndev->chip_name,
		local[0], local[1], local[2], local[3],
		mask[0], mask[1], mask[2], mask[3],
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
		link.connected ? "yes" : "no", speed,
		stat.rx_packets, stat.rx_errors,
		stat.tx_packets, stat.tx_errors);

	return 0;
}

// fix the world!
int main(int argc, char *argv[])
{
	__u32 local_ip, local_mask;
	__u8 mac[MAC_ADR_LEN];
	char ip[IPV4_STR_LEN], mask[IPV4_STR_LEN], mac_str[MAC_STR_LEN];
	__u32 flag = 0;
	int opt;
	struct net_device *ndev;
	struct list_node *iter, *ndev_list;

	ndev_list = ndev_get_list();
	if (list_is_empty(ndev_list)) {
		printf("No net device found!\n");
		return -ENODEV;
	}

	if (argc == 1) {
		list_for_each(iter, ndev_list) {
			ndev = container_of(iter, struct net_device, ndev_node);
			show_net_info(ndev);
			putchar('\n');
		}

		return 0;
	}

	ndev = container_of(ndev_list->next, struct net_device, ndev_node);

	while ((opt = getopt(argc, argv, "n:m:sh")) != -1) {
		switch (opt) {
		case 'n':
			if (str_to_ip((__u8*)&local_mask, optarg)) {
				printf("Invalid netmask \"%s\"!\n", optarg);
				return -EINVAL;
			}

			flag |= FLAG_MASK;
			break;

		case 'm':
			if (str_to_mac(mac, optarg)) {
				printf("Invalid MAC address \"%s\"!\n", optarg);
				return -EINVAL;
			}

			flag |= FLAG_MAC;
			break;

		case 's':
			flag |= FLAG_CFG;
			break;

		case 'h':
		default:
			usage();
			break;
		}
	}

	if (str_to_ip((__u8*)&local_ip, argv[optind]) == 0) {
		flag |= FLAG_IP;
	} else {
		list_for_each(iter, ndev_list) {
			ndev = container_of(iter, struct net_device, ndev_node);
			if (0 == strcmp(ndev->ifx_name, argv[optind]))
				break;
		}
		if (iter == ndev_list) {
			printf("The %s device not found!\n", argv[optind]);
			return -ENODEV;
		}

		optind++;

		if (optind < argc && str_to_ip((__u8*)&local_ip, argv[optind]) == 0)
			flag |= FLAG_IP;
	}

	if (flag & FLAG_IP)
		ndev_ioctl(ndev, NIOC_SET_IP, (void*)local_ip);

	if (flag & FLAG_MAC)
		ndev_ioctl(ndev, NIOC_SET_MAC, (void*)mac);

	if (flag & FLAG_MASK)
		ndev_ioctl(ndev, NIOC_SET_MASK, (void*)local_mask);

	if (flag & FLAG_CFG) {
		if (flag & FLAG_IP) {
			ip_to_str(ip, local_ip);
			conf_set_attr("net.eth0.address", ip);
		}

		if (flag & FLAG_MASK) {
			ip_to_str(mask, local_mask);
			conf_set_attr("net.eth0.address", mask);
		}

		if (flag & FLAG_MAC) {
			sprintf(mac_str, "%x.%02x.%02x.%02x%.02x.%02",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			conf_set_attr("net.eth0.mac", mac_str);
		}

		conf_store();
	}

	show_net_info(ndev);

	return 0;
}
