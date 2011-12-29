#include <autoconf.h>
#include <net/net.h>
#include <net/mii.h>
#include <irq.h>
#include <platform.h>
#include "lan9220.h"

struct lan9220_chip {
	void *iomem;
#ifdef CONFIG_IRQ_SUPPORT
	__u32 int_mask;
#endif
#if 1
	int busw32;
#else
	__u32 (*readl)(lan9220_chip *, int);
	__u32 (*writel)(lan9220_chip *, int, __u32);
#endif
};

static inline __u32 lan9220_readl(struct lan9220_chip *lan9220, __u8 reg)
{
	if (lan9220->busw32)
		return readl(lan9220->iomem + reg);

	return readw(lan9220->iomem + reg + 2) << 16 | readw(lan9220->iomem + reg);
}

static inline void lan9220_writel(struct lan9220_chip *lan9220, __u8 reg, __u32 val)
{
	if (lan9220->busw32) {
		writel(lan9220->iomem + reg, val);
	} else {
		writew(lan9220->iomem + reg, val & 0xffff);
		writew(lan9220->iomem + reg + 2, val >> 16 & 0xffff);
	}
}

static inline __u32 lan9220_csr_readl(struct lan9220_chip *lan9220, __u32 csr)
{
	lan9220_writel(lan9220, MAC_CSR_CMD, 1 << 31 | 1 << 30 | csr);
	while (lan9220_readl(lan9220, MAC_CSR_CMD) & 1 << 31);
	return lan9220_readl(lan9220, MAC_CSR_DATA);
}

static inline void lan9220_csr_writel(struct lan9220_chip *lan9220, __u32 csr, __u32 val)
{
	lan9220_writel(lan9220, MAC_CSR_DATA, val);
	lan9220_writel(lan9220, MAC_CSR_CMD, 1 << 31 | csr);
	while (lan9220_readl(lan9220, MAC_CSR_CMD) & 1 << 31);
}

static __u16 lan9220_mdio_read(struct net_device *ndev, __u8 addr, __u8 reg)
{
	struct lan9220_chip *lan9220 = ndev->chip;

	lan9220_csr_writel(lan9220, MII_ACC, addr << 11 | reg << 6 | 1);
	while (lan9220_csr_readl(lan9220, MII_ACC) & 0x1);
	return lan9220_csr_readl(lan9220, MII_DATA) & 0xffff;
}

static void lan9220_mdio_write(struct net_device *ndev, __u8 addr, __u8 reg, __u16 val)
{
	struct lan9220_chip *lan9220 = ndev->chip;

	lan9220_csr_writel(lan9220, MII_DATA, val);
	lan9220_csr_writel(lan9220, MII_ACC, addr << 11 | reg << 6 | 1 << 1 | 1);
	while (lan9220_csr_readl(lan9220, MII_ACC) & 0x1);
}

static int lan9220_hw_init(struct net_device *ndev)
{
	__u32 val;
#ifdef CONFIG_IRQ_SUPPORT // fixme
	struct mii_phy *phy;
#endif
	struct lan9220_chip *lan9220 = ndev->chip;

	// reset PHY
	val = lan9220_readl(lan9220, PMT_CTRL);
	lan9220_writel(lan9220, PMT_CTRL, val | 0x1 << 10);
	while (lan9220_readl(lan9220, PMT_CTRL) & 0x1 << 10);

	udelay(200);
	// while (lan9220_readl(lan9220, PMT_CTRL) & 0x1);

	// reset MAC CSR
	val = lan9220_readl(lan9220, HW_CFG);
	lan9220_writel(lan9220, HW_CFG, val | 0x1);
	while (lan9220_readl(lan9220, HW_CFG) & 0x1);

	// IRQ setting
#ifdef CONFIG_IRQ_SUPPORT
	lan9220_writel(lan9220, INT_EN, lan9220->int_mask);

	if (list_is_empty(&ndev->phy_list))
		return -EIO;
	phy = container_of(ndev->phy_list.next, struct mii_phy, phy_node);
	ndev->mdio_write(ndev, phy->addr,
		MII_REG_INT_MASK, PHY_INT_AN | PHY_INT_LINK);

	val = lan9220_readl(lan9220, IRQ_CFG);
	lan9220_writel(lan9220, IRQ_CFG, val | 0x1 << 8);
#endif

	// enable RX and TX
	val = lan9220_csr_readl(lan9220, MAC_CR);
	lan9220_csr_writel(lan9220, MAC_CR, val | 0x3 << 2);

	return 0;
}

static int lan9220_send_packet(struct net_device *ndev, struct sock_buff *skb)
{
	int i;
	__u32 cmd_A, cmd_B, status;
	__u32 *data;
	__u32 __UNUSED__ psr;
	struct lan9220_chip *lan9220 = ndev->chip;

	lock_irq_psr(psr);

	cmd_A = (1 << 13) | (1 << 12) | (skb->size & 0x7ff);
	cmd_B = skb->size & 0x7ff;

	lan9220_writel(lan9220, TX_DATA_PORT, cmd_A);
	lan9220_writel(lan9220, TX_DATA_PORT, cmd_B);

	data = (__u32 *)skb->data;
	for (i = 0; i < skb->size; i += 4, data++)
		lan9220_writel(lan9220, TX_DATA_PORT, *data);

	status = lan9220_readl(lan9220, TX_CFG);
	lan9220_writel(lan9220, TX_CFG, status | 0x1 << 1);
	while (lan9220_readl(lan9220, TX_CFG) & 0x1);

	status = lan9220_readl(lan9220, TX_STATUS_PORT);

	ndev->stat.tx_packets++;

	unlock_irq_psr(psr);

	return 0;
}

static int lan9220_recv_packet(struct net_device *ndev)
{
	int i;
	__u32 info_status, packet_status;
	__u32 packet_count, packet_length, packet_length_pad;
	__u32 *data;
	struct sock_buff *skb;
	struct lan9220_chip *lan9220 = ndev->chip;

	info_status = lan9220_readl(lan9220, RX_FIFO_INF);
	packet_count = info_status >> 16 & 0xff;

	while (packet_count--) {
		packet_status = lan9220_readl(lan9220, RX_STATUS_PORT);
		// fixme: to discard the error packet
		packet_length = packet_status >> 16 & 0x3fff;
		if (0 == packet_length)
			break;

		packet_length_pad = packet_length;
		ALIGN_UP(packet_length_pad, 4);

		skb = skb_alloc(0, packet_length_pad);
		if (NULL == skb)
			return -ENOMEM;

		skb->size = packet_length;
		data = (__u32 *)skb->data;

		for (i = 0; i < packet_length_pad; i += 4, data++)
			*data = lan9220_readl(lan9220, RX_DATA_PORT);

		skb->size -= 4;
		netif_rx(skb);

		ndev->stat.rx_packets++;
	}

	return 0;
}

static int lan9220_link_change(struct net_device *ndev)
{
	__u16 reg_src, reg_bms;
	struct mii_phy *phy;

	if (list_is_empty(&ndev->phy_list))
		return -EIO;
	phy = container_of(ndev->phy_list.next, struct mii_phy, phy_node);

	reg_src = ndev->mdio_read(ndev, phy->addr, MII_REG_INT_SRC);
	if (reg_src & (PHY_INT_AN | PHY_INT_LINK)) {
		reg_bms = ndev->mdio_read(ndev, phy->addr, MII_REG_BMS);

		if (reg_bms & (1 << 2)) {
			__u16 link;

			ndev->link.connected = true;

			link  = ndev->mdio_read(ndev, phy->addr, MII_REG_SPCS);
			switch ((link >> 2) & 0x7) {
			case 1:
				ndev->link.speed = ETHER_SPEED_10M_HD;
				break;

			case 5:
				ndev->link.speed = ETHER_SPEED_10M_FD;
				break;

			case 2:
				ndev->link.speed = ETHER_SPEED_100M_HD;
				break;

			case 6:
				ndev->link.speed = ETHER_SPEED_100M_FD;
				break;

			default:
				ndev->link.speed = ETHER_SPEED_UNKNOWN;
				break;
			}
		} else {
			ndev->link.connected = false;
		}

		ndev_link_change(ndev);
	}
	//

	return 0;
}

#ifdef CONFIG_IRQ_SUPPORT
static int lan9220_isr(__u32 irq, void *dev)
{
	__u32 status;
	struct net_device *ndev = (struct net_device *)dev;
	struct lan9220_chip *lan9220 = ndev->chip;

	// IRQ status issue
	status = lan9220_readl(lan9220, INT_STS);
#if 1
	printf("%s() line %d status = 0x%08x\n", __func__, __LINE__, status);
#endif
	status &= lan9220->int_mask;
	if (0 == status)
		return IRQ_NONE;
	lan9220_writel(lan9220, INT_STS, status);

	if (status & (INT_RXD | INT_RSFL))
		lan9220_recv_packet(ndev);

	if (status & INT_PHY)
		lan9220_link_change(ndev);

	return IRQ_HANDLED;
}
#else
static int lan9220_poll(struct net_device *ndev)
{
	// fixme
	lan9220_link_change(ndev);
	return lan9220_recv_packet(ndev);
}
#endif

static int lan9220_set_mac(struct net_device *ndev, const __u8 *mac)
{
	struct lan9220_chip *lan9220 = ndev->chip;

	lan9220_csr_writel(lan9220, ADDRL, *(__u32 *)mac);
	lan9220_csr_writel(lan9220, ADDRH, *(__u16 *)(mac + 4));
	return 0;
}

static int __INIT__ lan9220_init(struct platform_device *plat_dev)
{
	int ret;
	__u16 id[2];
	const char *chip_name;
	int irq;
	unsigned long mem;
	struct net_device *ndev;
	struct lan9220_chip *lan9220;

	mem = platform_get_memory(plat_dev);
	irq = platform_get_irq(plat_dev);

	ndev = ndev_new(sizeof(*lan9220));
	if (NULL == ndev)
		return -ENOMEM;

	lan9220 = ndev->chip;
	lan9220->iomem = VA(mem);
#ifdef CONFIG_IRQ_SUPPORT
	lan9220->int_mask = 0xfffff07f;
#endif

	// id = readl(ID_REV);
	id[0] = readw(lan9220->iomem + ID_REV); // REV
	id[1] = readw(lan9220->iomem + ID_REV + 2); // Device ID

	switch (id[1]) {
	case DEV_ID_LAN9118:
		lan9220->busw32 = 1;
		chip_name = "LAN9118 (32-bit)";
		break;

	case DEV_ID_LAN9220:
		lan9220->busw32 = 0;
		chip_name = "LAN9220 (16-bit)";
		break;

	default:
		printf("unknown device id (0x%04x)!\n", id[1]);
		ret = -ENODEV;
		goto error;
	}

	printf("%s found, rev = 0x%04x\n", chip_name, id[0]);

	ndev->chip_name    = chip_name;
	ndev->set_mac_addr = lan9220_set_mac;
	ndev->send_packet  = lan9220_send_packet;
#ifndef CONFIG_IRQ_SUPPORT
	ndev->ndev_poll    = lan9220_poll;
#endif

	// MII
	ndev->phy_mask   = 2;
	ndev->mdio_read  = lan9220_mdio_read;
	ndev->mdio_write = lan9220_mdio_write;

	ret = ndev_register(ndev);
	if (ret < 0)
		goto error;

#ifdef CONFIG_IRQ_SUPPORT
	if (id[1] == DEV_ID_LAN9118)
		irq_set_trigger(irq, IRQ_TYPE_LOW);
	else
		irq_set_trigger(irq, IRQ_TYPE_HIGH);

	ret = irq_register_isr(irq, lan9220_isr, ndev);
	if (ret < 0)
		goto error;
#endif

	ret = lan9220_hw_init(ndev);
	if (!ret)
		return 0;

	// TODO: deinit and unregister the ndev
error:
	free(ndev);
	return ret;
}

static struct platform_driver lan9220_driver = {
	.drv = {
		.name = "SMSC LAN9220",
	},
	.init = lan9220_init,
};

static int __INIT__ lan9220_driver_init(void)
{
	return platform_driver_register(&lan9220_driver);
}

DRIVER_INIT(lan9220_driver_init);
