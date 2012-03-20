#include <io.h>
#include <init.h>
#include <errno.h>
#include <delay.h>
#include <stdio.h>
#include <net/net.h>
#include <net/mii.h>
#include "smsc91x.h"

static inline void smsc91x_switch_bank(__u16 reg)
{
	writew(VA(SMSC91X_BASE + 0xe), 0x3300 + reg);
}

static inline __u32 smsc91x_readl(__u16 reg)
{
	return readl(VA(SMSC91X_BASE + reg));
}

static inline __u16 smsc91x_read(__u16 reg)
{
	return readw(VA(SMSC91X_BASE + reg));
}

static inline void smsc91x_write(__u16 reg, __u16 val)
{
	writew(VA(SMSC91X_BASE + reg), val);
}

static inline void smsc91x_writel(__u16 reg, __u32 val)
{
	writel(VA(SMSC91X_BASE + reg), val);
}

static int smsc91x_recv_packet(struct net_device *ndev)
{
	int i;
	__u16 pack_num, size, *data;
	// __u16 status;
	struct sock_buff *skb;

	smsc91x_switch_bank(0x2);

	pack_num = smsc91x_read(0x4);
	if (pack_num & 0x8000)
		return 0;

	smsc91x_write(0x6, 1 << 15 | 1 << 14 | 1 << 13);
	// status = smsc91x_read(0x8);
	size   = smsc91x_read(0x8) & 0x7ff;

	if (!size)
		return 0;

	skb = skb_alloc(0, size);
	if (!skb)
		return -ENOMEM;

	data = (__u16 *)skb->data;
	for (i = 0; i < (size >> 1); i++)
		data[i] = smsc91x_read(0x8);

	// release the rx buffer
	while (smsc91x_read(0x0) & 0x1);
	smsc91x_write(0x0, 0x4 << 5);
	while (smsc91x_read(0x0) & 0x1);

	ndev->stat.rx_packets++;

	netif_rx(skb);

	return 0;
}

static int smsc91x_send_packet(struct net_device *ndev, struct sock_buff *skb)
{
	int i;
	__u32 size;
	__u16 *data;
	__UNUSED__ __u32 psr;

	lock_irq_psr(psr);

	// 4 CRC bytes and 2 bytes control bytes
	size = skb->size + 6;
	data = (__u16 *)(skb->data);

	smsc91x_switch_bank(0x2);

	smsc91x_write(0x6, 0x1 << 14);

	// write size
	smsc91x_writel(0x8, size << 16);
	for (i = 0; i < (size >> 1); i++)
		smsc91x_write(0x8, data[i]);

	smsc91x_write(0x0, 0x6 << 5);
	while (smsc91x_read(0x0) & 0x1);

	ndev->stat.tx_packets++;

	lock_irq_psr(psr);

	return size;
}

static int smsc91x_set_mac(struct net_device *ndev, const __u8 mac[])
{
	int i;

	smsc91x_switch_bank(0x1);

	for (i = 0; i < 6; i++)
		smsc91x_write(0x4 + i, ndev->mac_addr[i]);

	return 0;
}

static int smsc91x_poll(struct net_device *ndev)
{
	return smsc91x_recv_packet(ndev);
}

static int smsc91x_hw_init(void)
{
	__u16 val;

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
		printf("The link status down!\n");
	else
		printf("The link status up!\n");

	return 0;

}

static int __INIT__ smsc91x_init(void)
{
	int ret;
	__u16 chip_id;
	struct net_device *ndev;

	// probe chip
	smsc91x_switch_bank(0x3);

	chip_id = smsc91x_read(0xa);
	if (chip_id != SMSC91C111_ID) {
		printf("SMSC91X Ethernet not found!\n");
		return -ENODEV;
	}

	printf("SMSC91X ID = 0x%x\n", chip_id);

	ret = smsc91x_hw_init();
	if (ret < 0) {
		printf("smsc hw init failed!\n");
		return ret;
	}

	ndev = ndev_new(0);
	if (!ndev)
		return -ENOMEM;

	ndev->chip_name    = "LAN91C111";
	ndev->set_mac_addr = smsc91x_set_mac;
	ndev->send_packet  = smsc91x_send_packet;
#ifndef CONFIG_IRQ_SUPPORT
	ndev->ndev_poll    = smsc91x_poll; // only polling mode supported so far
#endif

	ret = ndev_register(ndev);

	return ret;
}

module_init(smsc91x_init);
