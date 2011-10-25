#include <sysconf.h>
#include <getopt.h>

static int show_net_info(struct net_device *ndev)
{
	__u8 local[IPV4_ADR_LEN];
	__u8 mask[IPV4_ADR_LEN];
	__u8 mac[MAC_ADR_LEN];
	struct link_status net_status;
	const char *speed = "none";

	ndev_ioctl(ndev, NIOC_GET_IP, local);
	ndev_ioctl(ndev, NIOC_GET_MASK, mask);
	ndev_ioctl(ndev, NIOC_GET_MAC, mac);
	ndev_ioctl(ndev, NIOC_GET_STATUS, &net_status);

	if (net_status.connected) {
		switch (net_status.link_speed) {
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
			break;
		}
	}

	printf("%-8schip name:%s\n"
			"\tlocal addr: %d.%d.%d.%d\n"
			"\tlocal mask: %d.%d.%d.%d\n"
			"\tMAC addr: %02x:%02x:%02x:%02x:%02x:%02x\n"
			"\tconnection:%s speed:%s\n"
			"\tRX packets:%d errors:%d\n"
			"\tTX packets:%d errors:%d\n",
			ndev->ifx_name,
			ndev->chip_name,
			local[0], local[1], local[2], local[3],
			mask[0], mask[1], mask[2], mask[3],
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
			net_status.connected ? "yes" : "no", speed,
			ndev->stat.rx_packets, ndev->stat.rx_errors,
			ndev->stat.tx_packets, ndev->stat.tx_errors);

	return 0;
}

int main(int argc, char *argv[])
{
	__u32 local_ip, local_mask;
	__u8 mac[MAC_ADR_LEN];
	int flag_ip, flag_mask, flag_mac, flag_conf;
	struct net_device *ndev;
	struct list_node *iter, *ndev_list;
	int opt;
	char ip[IPV4_STR_LEN];
	char mask[IPV4_STR_LEN];

	ndev_list = net_get_device_list();
	if (list_is_empty(ndev_list)) {
		printf("There're no net device found!\n");
		return -ENODEV;
	}

	if (argc == 1) {
		list_for_each(iter, ndev_list) {
			ndev = container_of(iter, struct net_device, ndev_node);
			show_net_info(ndev);
		}

		return 0;
	}

	ndev = container_of(ndev_list->next, struct net_device, ndev_node);

	flag_ip = flag_mask = flag_mac = flag_conf = 0;
	while ((opt = getopt(argc, argv, "n:m:sh")) != -1) {
		switch (opt) {
		case 'n':
			if (str_to_ip((__u8*)&local_mask, optarg)) {
				printf("Invalid netmask!\n");
				return -EINVAL;
			}

			flag_mask = 1;
			break;

		case 'm':
			if (str_to_mac(mac, optarg)) {
				printf("Invalid MAC address!\n");
				return -EINVAL;
			}

			flag_mac = 1;
			break;

		case 's':
			flag_conf = 1;
			break;

		case 'h':
		default:
			usage();
			break;
		}
	}

	if (str_to_ip((__u8*)&local_ip, argv[optind]) == 0) {
		flag_ip = 1;
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
			flag_ip = 1;
	}


	if (flag_ip)
		ndev_ioctl(ndev, NIOC_SET_IP, (void*)local_ip);

	if (flag_mac)
		ndev_ioctl(ndev, NIOC_SET_MAC, (void*)mac);

	if (flag_mask)
		ndev_ioctl(ndev, NIOC_SET_MASK, (void*)local_mask);

	if (flag_conf) {
		if (flag_ip) {
			ip_to_str(ip, local_ip);
			conf_set_attr("net.eth0.address", ip);
		}

		if (flag_mask) {
			ip_to_str(mask, local_mask);
			conf_set_attr("net.eth0.address", mask);
		}

		if (flag_mac)
			conf_set_attr("net.eth0.mac", (char *)mac);

		conf_store();
	}

	show_net_info(ndev);

	return 0;
}
