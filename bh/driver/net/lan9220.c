#include <autoconf.h>
#include <net/net.h>
#include <net/mii.h>
#include <irq.h>
#include "lan9220.h"

#if 0
struct lan9220_chip {
	int lan9220_32bits;
	__u32 (*readl)(lan9220_chip *, int);
	__u32 (*writel)(lan9220_chip *, int, __u32);
	// ...
};
#else
static int lan9220_32bits;
#endif

// fixme
#ifdef CONFIG_IRQ_SUPPORT
static __u32 int_mask = 0xfffff07f;
#endif

static inline __u32 lan9220_readl(__u8 reg)
{
	if (lan9220_32bits)
		return readl(VA(LAN9220_BASE + reg));

	return readw(VA(LAN9220_BASE + reg + 2)) << 16 | readw(VA(LAN9220_BASE + reg));
}

static inline void lan9220_writel(__u8 reg, __u32 val)
{
	if (lan9220_32bits) {
		writel(VA(LAN9220_BASE + reg), val);
	} else {
		writew(VA(LAN9220_BASE + reg), val & 0xffff);
		writew(VA(LAN9220_BASE + reg + 2), val >> 16 & 0xffff);
	}
}

static inline __u32 lan9220_csr_readl(__u32 csr)
{
	lan9220_writel(MAC_CSR_CMD, 1 << 31 | 1 << 30 | csr);
	while (lan9220_readl(MAC_CSR_CMD) & 1 << 31);
	return lan9220_readl(MAC_CSR_DATA);
}

static inline void lan9220_csr_writel(__u32 csr, __u32 val)
{
	lan9220_writel(MAC_CSR_DATA, val);
	lan9220_writel(MAC_CSR_CMD, 1 << 31 | csr);
	while (lan9220_readl(MAC_CSR_CMD) & 1 << 31);
}

static __u16 lan9220_mdio_read(struct net_device *ndev, __u8 addr, __u8 reg)
{
	lan9220_csr_writel(MII_ACC, addr << 11 | reg << 6 | 1);
	while (lan9220_csr_readl(MII_ACC) & 0x1);
	return lan9220_csr_readl(MII_DATA) & 0xffff;
}

static void lan9220_mdio_write(struct net_device *ndev, __u8 addr, __u8 reg, __u16 val)
{
	lan9220_csr_writel(MII_DATA, val);
	lan9220_csr_writel(MII_ACC, addr << 11 | reg << 6 | 1 << 1 | 1);
	while (lan9220_csr_readl(MII_ACC) & 0x1);
}

static int lan9220_hw_init(struct net_device *ndev)
{
	__u32 val;
#ifdef CONFIG_IRQ_SUPPORT
	struct mii_phy *phy;
#endif

	// reset PHY
	val = lan9220_readl(PMT_CTRL);
	lan9220_writel(PMT_CTRL, val | 0x1 << 10);
	while (lan9220_readl(PMT_CTRL) & 0x1 << 10);

	udelay(200);
	// while (lan9220_readl(PMT_CTRL) & 0x1);

	// reset MAC CSR
	val = lan9220_readl(HW_CFG);
	lan9220_writel(HW_CFG, val | 0x1);
	while (lan9220_readl(HW_CFG) & 0x1);

	// IRQ setting
#ifdef CONFIG_IRQ_SUPPORT
	lan9220_writel(INT_EN, int_mask);

	if (list_is_empty(&ndev->phy_list))
		return -EIO;
	phy = container_of(ndev->phy_list.next, struct mii_phy, phy_node);
	ndev->mdio_write(ndev, phy->mii_id,
		MII_REG_INT_MASK, PHY_INT_AN | PHY_INT_LINK);

	val = lan9220_readl(IRQ_CFG);
	lan9220_writel(IRQ_CFG, val | 0x1 << 8);
#endif

	// enable RX and TX
	val = lan9220_csr_readl(MAC_CR);
	lan9220_csr_writel(MAC_CR, val | 0x3 << 2);

	return 0;
}

static int lan9220_send_packet(struct net_device *ndev, struct sock_buff *skb)
{
	int i;
	__u32 cmd_A, cmd_B, status;
	__u32 *data;
	__u32 __UNUSED__ psr;

	lock_irq_psr(psr);

	cmd_A = (1 << 13) | (1 << 12) | (skb->size & 0x7ff);
	cmd_B = skb->size & 0x7ff;

	lan9220_writel(TX_DATA_PORT, cmd_A);
	lan9220_writel(TX_DATA_PORT, cmd_B);

	data = (__u32 *)skb->data;
	for (i = 0; i < skb->size; i += 4, data++)
		lan9220_writel(TX_DATA_PORT, *data);

	status = lan9220_readl(TX_CFG);
	lan9220_writel(TX_CFG, status | 0x1 << 1);
	while (lan9220_readl(TX_CFG) & 0x1);

	status = lan9220_readl(TX_STATUS_PORT);

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

	info_status = lan9220_readl(RX_FIFO_INF);
	packet_count = info_status >> 16 & 0xff;

	while (packet_count--) {
		packet_status = lan9220_readl(RX_STATUS_PORT);
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
			*data = lan9220_readl(RX_DATA_PORT);

		skb->size -= 4;
		netif_rx(skb);

		ndev->stat.rx_packets++;
	}

	return 0;
}

#ifdef CONFIG_IRQ_SUPPORT
static int lan9220_isr(__u32 irq, void *dev)
{
	__u32 status;
	struct mii_phy *phy;
	struct net_device *ndev = (struct net_device *)dev;

#if 0
	// IRQ status issue
	status = lan9220_readl(INT_STS);
	printf("%s() line %d status = 0x%08x\n", __func__, __LINE__, status);
	status &= int_mask;
#else
	status = lan9220_readl(INT_STS) & int_mask;
#endif
	if (0 == status)
		return IRQ_NONE;
	lan9220_writel(INT_STS, status);

	if (status & (INT_RXD | INT_RSFL))
		lan9220_recv_packet(ndev);

	if (status & INT_PHY) {
		__u16 reg_src, reg_bms;

		if (list_is_empty(&ndev->phy_list))
			return -EIO;
		phy = container_of(ndev->phy_list.next, struct mii_phy, phy_node);

		reg_src = ndev->mdio_read(ndev, phy->mii_id, MII_REG_INT_SRC);
		if (reg_src & (PHY_INT_AN | PHY_INT_LINK)) {
			reg_bms = ndev->mdio_read(ndev, phy->mii_id, MII_REG_BMS);
			// fixme: to be moved to up layer
			printf("link %s, speed = %s\n",
				reg_bms & (1 << 2) ? "up" : "down",
				reg_bms & (3 << 13) ? "100M" : "10M");
		}
	}

	return IRQ_HANDLED;
}
#else
static int lan9220_poll(struct net_device *ndev)
{
	return lan9220_recv_packet(ndev);
}
#endif

static int lan9220_set_mac(struct net_device *ndev, const __u8 *mac)
{
	lan9220_csr_writel(ADDRL, *(__u32 *)mac);
	lan9220_csr_writel(ADDRH, *(__u16 *)(mac + 4));
	return 0;
}

static int __INIT__ lan9220_init(struct device *dev)
{
	int ret;
	__u32 mac_id;
	const char *chip_name;
	struct net_device *ndev;

	ndev = ndev_new(0);
	if (NULL == ndev)
		return -ENOMEM;

	ndev->dev = dev;

	mac_id = lan9220_readl(ID_REV);

	switch (mac_id >> 16) {
	case PID_LAN9118:
		lan9220_32bits = 1;
		chip_name = "LAN9118 (32-bit)";
		break;

	case PID_LAN9220:
		lan9220_32bits = 0;
		chip_name = "LAN9220 (16-bit)";
		break;

	default:
		// chip_name = "Unknown";
		// break;
		return -ENODEV; // fixme
	}

	printf("%s found, rev = 0x%04x\n", chip_name, mac_id & 0xFFFF);

	ndev->chip_name = chip_name;
	ndev->set_mac_addr = lan9220_set_mac;
	ndev->send_packet = lan9220_send_packet;
#ifndef CONFIG_IRQ_SUPPORT
	ndev->ndev_poll = lan9220_poll;
#endif
	ndev->mdio_read  = lan9220_mdio_read;
	ndev->mdio_write = lan9220_mdio_write;

	ret = ndev_register(ndev);
	if (ret < 0)
		goto error;

#ifdef CONFIG_IRQ_SUPPORT
	// fixme
	if ((mac_id >> 16) == PID_LAN9118)
		writel(VA(GPIO1_BASE + LEVELDETECT1), 1 << 19);
	else
		writel(VA(GPIO1_BASE + LEVELDETECT0), 1 << 19);

	ret = irq_register_isr(LAN9220_IRQ_NUM, lan9220_isr, ndev);
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

static struct driver lan9220_driver = {
	.name = "SMSC LAN9220",
	.init = lan9220_init,
};

static int __INIT__ lan9220_driver_init(void)
{
	return driver_register(&lan9220_driver);
}

DRIVER_INIT(lan9220_driver_init);
