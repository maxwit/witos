#include <sysconf.h>
#include <getopt.h>

static int show_net_info(struct net_device *ndev)
{
	u8 local[IPV4_ADR_LEN];
	u8 mask[IPV4_ADR_LEN];
	u8 mac[MAC_ADR_LEN];
	struct link_status net_status;
	const char *speed = "none";

	ndev_ioctl(ndev, NIOC_GET_IP, local);
	ndev_ioctl(ndev, NIOC_GET_MASK, mask);
	ndev_ioctl(ndev, NIOC_GET_MAC, mac);
	ndev_ioctl(ndev, NIOC_GET_STATUS, &net_status);

	if (net_status.connected)
	{
		switch (net_status.link_speed)
		{
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
	int i;
	u32 local_ip, local_mask;
	u8 mac[MAC_ADR_LEN];
	struct net_device *ndev;
	struct list_node *iter, *ndev_list;

	ndev_list = net_get_device_list();
	if (list_is_empty(ndev_list))
	{
		printf("There're no net device found!\n");
		return -ENODEV;
	}

	if (argc == 1)
	{
		list_for_each(iter, ndev_list)
		{
			ndev = container_of(iter, struct net_device, ndev_node);
			show_net_info(ndev);
		}

		return 0;
	}

	ndev = container_of(ndev_list->next, struct net_device, ndev_node);

	for (i = 1; i < argc; i++)
	{
		if (strlen(argv[i]) >= 3)
		{
			if (argv[i][0] == 'e' && argv[i][1] == 't' && argv[i][2] == 'h')
			{
				list_for_each(iter, ndev_list)
				{
					ndev = container_of(iter, struct net_device, ndev_node);
					if (0 == strcmp(ndev->ifx_name, argv[i]))
						break;
				}
				if (iter == ndev_list)
				{
					printf("The %s device not found!\n", argv[i]);
					return -ENODEV;
				}
				continue;
			}
		}

		if (str_to_ip((u8*)&local_ip, argv[i]) == 0)
		{
			ndev_ioctl(ndev, NIOC_SET_IP, (void*)local_ip);
			continue;
		}

		if (strcmp(argv[i], "netmask") == 0)
		{
			if (++i == argc || str_to_ip((u8*)&local_mask, argv[i]))
			{
				printf("Invalid netmask!\n");
				return -EINVAL;
			}
			ndev_ioctl(ndev, NIOC_SET_MASK, (void*)local_mask);
			continue;
		}

		if (strcmp(argv[i], "hw") == 0 || strcmp(argv[i], "HW") == 0)
		{
			if (++i == argc || str_to_mac(mac, argv[i]))
			{
				printf("Invalid MAC address!\n");
				return -EINVAL;
			}
			ndev_ioctl(ndev, NIOC_SET_MAC, (void*)mac);
			continue;
		}
	}

	show_net_info(ndev);

	//fixme
	conf_store();

	return 0;
}
