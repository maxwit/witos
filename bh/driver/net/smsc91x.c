#include <stdio.h>
#include <net/net.h>
#include <net/mii.h>
#include "smsc91x.h"

static inline void smsc91x_switch_bank(u16 reg)
{
	writew(VA(SMSC91X_BASE + 0xe), 0x3300 + reg);
}

static inline u32 smsc91x_readl(u16 reg)
{
	return readl(VA(SMSC91X_BASE + reg));
}

static inline u16 smsc91x_read(u16 reg)
{
	return readw(VA(SMSC91X_BASE + reg));
}

static inline void smsc91x_write(u16 reg, u16 val)
{
	writew(VA(SMSC91X_BASE + reg), val);
}

static inline void smsc91x_writel(u16 reg, u32 val)
{
	writel(VA(SMSC91X_BASE + reg), val);
}

static int smsc91x_recv_packet(struct net_device *ndev)
{
	u16 pack_num, status, len, *data;
	u32  i;
	struct sock_buff *skbuf;

	smsc91x_switch_bank(0x2);

	pack_num = smsc91x_read(0x4);
	if (pack_num & 0x8000)
	{
		return 0;
	}

	smsc91x_write(0x6, 1 << 15 | 1 << 14 | 1 << 13);
	status = smsc91x_read(0x8);
	len    = smsc91x_read(0x8) & 0x7ff;

	if (!len)
	{
		return 0;
	}

	//len is always even
	skbuf =skb_alloc(0, len);
	skbuf->size = len;

	data = (u16 *)skbuf->data;

	for (i = 0; i < (len >> 1); i++)
	{
		data[i] = smsc91x_read(0x8);
	}

	//release the rx buffer
	while (smsc91x_read(0x0) & 0x1);
	smsc91x_write(0x0, 0x4 << 5);
	while (smsc91x_read(0x0) & 0x1);

	ndev->stat.rx_packets++;

	netif_rx(skbuf);

	return 1;
}

static int smsc91x_send_packet(struct net_device *ndev, struct sock_buff *skb)
{
	u32 len, i;
	u16 *data;

	// 4 CRC bytes and 2 bytes control bytes
	len  = skb->size + 6;
	data = (u16 *)(skb->data);

	smsc91x_switch_bank(0x2);

	smsc91x_write(0x6, 0x1 << 14);

	//write size
	smsc91x_writel(0x8, len << 16);
	for (i = 0; i < (len >> 1); i++)
	{
		smsc91x_write(0x8, data[i]);
	}

	smsc91x_write(0x0, 0x6 << 5);
	while (smsc91x_read(0x0) & 0x1);

	ndev->stat.tx_packets++;

	return len;
}

static int smsc91x_set_mac(struct net_device *ndev, const u8 mac[])
{
	int i;

	smsc91x_switch_bank(0x1);

	for (i = 0; i < 6; i++)
	{
		smsc91x_write(0x4 + i, ndev->mac_addr[i]);
	}

	return 0;
}

static int smsc91x_poll(struct net_device *ndev)
{
	return smsc91x_recv_packet(ndev);
}

static int smsc91x_hw_init(void)
{
	u16 val;

	smsc91x_switch_bank(0x1);
	//enable EPH Power
	val = smsc91x_read(0x0);
	val |= 0x1 << 15;
	smsc91x_write(0x0, val);

	//set PDN bit in PHY MI Register 0

	//reset mmu and alloc tx buffer
	smsc91x_switch_bank(0x2);
	smsc91x_write(0x0, 0x2 << 5);
	while (smsc91x_read(0x0) & 0x1);

	smsc91x_write(0x0, 0x1 << 5);
	while (smsc91x_read(0x0) & 0x1);
	udelay(0xffff);
	//fix me, if the alloc failed!

	smsc91x_switch_bank(0x0);
	//enable TX
	val = smsc91x_read(0x0);
	val |= 0x1;
	smsc91x_write(0x0, val);

	//enable RX
	val = smsc91x_read(0x4);
	val |=  0x1 << 9 | 0x1 << 8 | 0x1 << 2 | 0x1 << 1;
	smsc91x_write(0x4, val);

	//Read the Link status
	val = smsc91x_read(0x2);
	if (!(val & 0x4000))
	{
		printf("The link status is nok!\n");
		return -1;
	}

	printf("The link status is ok!\n", val);

	return 0;

}

static int __INIT__ smsc91x_init(void)
{
	int ret;
	u16 chip_id;
	struct net_device *ndev;

	// probe chip
	smsc91x_switch_bank(0x3);
	chip_id = smsc91x_read(0xa);

	if (chip_id != SMSC91C111_ID)
	{
		printf("SMSC91X Ethernet not found!\n");
		return -ENODEV;
	}
	printf("SMSC91X ID = 0x%x\n", chip_id);

	ret = smsc91x_hw_init();
	if (ret < 0)
	{
		printf("smsc hw init failed!\n");
		return ret;
	}

	ndev = ndev_new(0);
	if (!ndev)
	{
		return -ENOMEM;
	}

	ndev->chip_name    = "LAN91C111";
	ndev->set_mac_addr = smsc91x_set_mac;
	ndev->send_packet  = smsc91x_send_packet;
	ndev->ndev_poll    = smsc91x_poll; // only polling mode supported so far

	ret = ndev_register(ndev);

	return ret;
}

DRIVER_INIT(smsc91x_init);
