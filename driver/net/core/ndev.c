#include <init.h>
#include <delay.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <net/net.h>
#include <net/ndev.h>
#include <net/mii.h>

static DECL_INIT_LIST(g_ndev_list);
static int ndev_count = 0;

struct list_node *ndev_get_list(void)
{
	return &g_ndev_list;
}

#if 0
struct net_device *net_get_dev(const char *ifx)
{
	struct net_device *ndev;
	struct list_node *iter;

	list_for_each(iter, ndev_get_list())
	{
		ndev = container_of(iter, struct net_device, ndev_node);

		if (!strcmp(ifx, ndev->ifx_name))
			return ndev;
	}

	return NULL;
}
#endif

static int ndev_config(struct net_device *ndev)
{
	int ret;
	__u32 ip, netmask;
	__u8 mac[MAC_ADR_LEN];
	char buff[CONF_VAL_LEN];
	char attr[CONF_ATTR_LEN];

	// set IP address
	sprintf(attr, "net.%s.address", ndev->ifx_name);
	if (!conf_get_attr(attr, buff) && !str_to_ip((__u8 *)&ip, buff))
		ndev_ioctl(ndev, NIOC_SET_IP, (void *)ip);

	// set net mask
	sprintf(attr, "net.%s.netmask", ndev->ifx_name);
	if (!conf_get_attr(attr, buff) && !str_to_ip((__u8 *)&netmask, buff))
		ndev_ioctl(ndev, NIOC_SET_MASK, (void *)netmask);

	// set mac address
	sprintf(attr, "net.%s.mac", ndev->ifx_name);
	if (conf_get_attr(attr, buff) < 0 || str_to_mac(mac, buff) < 0) {
		int i;

		mac[0] = 0x20;
		mac[1] = 0x12;

		// make sure different ndev use different MAC address
		srandom(ndev_count + 1);
		for (i = 2; i < MAC_ADR_LEN; i++)
			mac[i] = (__u8)(random());
	}

	ret = ndev_ioctl(ndev, NIOC_SET_MAC, mac);

	return ret;
}
static int mii_bus_scan(struct net_device *ndev)
{
	int index, count = 0;
	struct mii_phy *phy;

	for (index = 0; index < 32; index++) {
		if (!((1 << index) & ndev->phy_mask))
			continue;

		phy = mii_phy_probe(ndev, index);
		if (phy) {
			phy->ndev = ndev;
			list_add_tail(&phy->phy_node, &ndev->phy_list);

			// does it work in any case?
			// mii_reset_phy(ndev, phy);

			printf("PHY @ MII[%d]: Vendor ID = 0x%04x, Device ID = 0x%04x\n",
				index, phy->ven_id, phy->dev_id);

			count++;
		}
	}

	return count;
}

int ndev_register(struct net_device *ndev)
{
	int ret;

	if (!ndev || !ndev->send_packet || !ndev->set_mac_addr)
		return -EINVAL;

	if (!ndev->chip_name)
		printf("Warning: chip_name is NOT set!\n");

	ret = ndev_config(ndev);
	if (ret < 0)
		return ret;

	if (ndev->phy_mask && ndev->mdio_read && ndev->mdio_write) {
		ret = mii_bus_scan(ndev);
		if (ret < 0)
			return ret;
	} else {
		printf("%s: MII does NOT support!\n", ndev->chip_name);
	}

	list_add_tail(&ndev->ndev_node, &g_ndev_list);

	return 0;
}

struct net_device *ndev_new(size_t chip_size)
{
	struct net_device *ndev;
	__u32 core_size = (sizeof(struct net_device) + WORD_SIZE - 1) & ~(WORD_SIZE - 1);

	ndev = zalloc(core_size + chip_size);
	if (NULL == ndev)
		return NULL;

	ndev->chip = (void *)ndev + core_size;
	ndev->phy_mask = 0xFFFFFFFF;
	ndev->link.connected = false;
	ndev->link.speed = ETHER_SPEED_UNKNOWN;

	// set default name
	snprintf(ndev->ifx_name, NET_NAME_LEN, "eth%d", ndev_count);
	ndev_count++;

	list_head_init(&ndev->ndev_node);
	list_head_init(&ndev->phy_list);

	return ndev;
}

#ifndef CONFIG_IRQ_SUPPORT
int ndev_poll()
{
	int ret = -ENODEV;
	struct list_node *iter;

	list_for_each(iter, &g_ndev_list) {
		struct net_device *ndev;

		ndev = container_of(iter, struct net_device, ndev_node);
		if (ndev->ndev_poll)
			ret = ndev->ndev_poll(ndev);
	}

	return ret;
}
#endif

struct net_device *ndev_get_first()
{
	struct list_node *first = g_ndev_list.next;

	if (!first)
		return NULL;

	return container_of(first, struct net_device, ndev_node);
}

int ndev_ioctl(struct net_device *ndev, int cmd, void *arg)
{
	assert(ndev);

	switch (cmd) {
	case NIOC_GET_IP:
		*(__u32 *)arg = ndev->ip;
		break;

	case NIOC_SET_IP:
		ndev->ip = (__u32)arg;
		break;

	case NIOC_GET_MASK:
		*(__u32 *)arg = ndev->mask;
		break;

	case NIOC_SET_MASK:
		ndev->mask = (__u32)arg;
		break;

	case NIOC_GET_MAC:
		memcpy(arg, ndev->mac_addr, MAC_ADR_LEN);
		break;

	case NIOC_SET_MAC:
		if (!ndev->set_mac_addr)
			return -EINVAL;

		memcpy(ndev->mac_addr, arg, MAC_ADR_LEN);
		ndev->set_mac_addr(ndev, arg);
		break;

	case NIOC_GET_LINK:
		*(struct link_status *)arg = ndev->link;
		break;

	case NIOC_GET_STAT:
		*(struct ndev_stat *)arg = ndev->stat;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

void ndev_link_change(struct net_device *ndev)
{
	printf("%s(\"%s\") link ", ndev->ifx_name, ndev->chip_name);

	if (ndev->link.connected == false)	{
		printf("down!\n");
	} else {
		printf("up, speed = ");

		switch (ndev->link.speed) {
		case ETHER_SPEED_10M_HD:
			printf("10M Half duplex!\n");
			break;

		case ETHER_SPEED_100M_FD:
			printf("100M Full duplex!\n");
			break;

		case ETHER_SPEED_1000M_FD:
			printf("1000M Full duplex!\n");
			break;

		// TODO:

		default:
			printf("Unknown!\n");
			break;
		}
	}
}
