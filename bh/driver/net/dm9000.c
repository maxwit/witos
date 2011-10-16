/*
 *  comment here
 */

#include <net/net.h>
#include <net/mii.h>
#include <irq.h>
#include "dm9000.h"

// TODO: support 16-bit BW

static u8 dm9000_readb(u8 reg)
{
	writeb(VA(DM9000_INDEX_PORT), reg);
	return readb(VA(DM9000_DATA_PORT));
}

static void dm9000_writeb(u8 reg, u8 val)
{
	writeb(VA(DM9000_INDEX_PORT), reg);
	writeb(VA(DM9000_DATA_PORT), val);
}

static u16 dm9000_mdio_read(struct net_device *ndev, u8 mii_id, u8 reg)
{
	u16 val;

	// dm9000_writeb(DM9000_EPAR, DM9000_PHY_INTER | reg);
	dm9000_writeb(DM9000_EPAR, (mii_id & 0x3) << 6 | reg);
	dm9000_writeb(DM9000_EPCR, DM9000_PHY_SELECT | DM9000_PHY_READ);

	while (dm9000_readb(DM9000_EPCR) & DM9000_PHY_BUSY);

	val = (dm9000_readb(DM9000_EPDRH) << 8) | dm9000_readb(DM9000_EPDRL);

	dm9000_writeb(DM9000_EPCR, 0x0);

	return val;
}

static void dm9000_mdio_write(struct net_device *ndev, u8 mii_id, u8 reg, u16 val)
{
	// dm9000_writeb(DM9000_EPAR, DM9000_PHY_INTER | reg);
	dm9000_writeb(DM9000_EPAR, (mii_id & 0x3) << 6 | reg);
	dm9000_writeb(DM9000_EPDRL, val & 0xff);
	dm9000_writeb(DM9000_EPDRH, (val >> 8) & 0xff);

	dm9000_writeb(DM9000_EPCR, DM9000_PHY_SELECT | DM9000_PHY_WRITE);

	while (dm9000_readb(DM9000_EPCR) & DM9000_PHY_BUSY);

	dm9000_writeb(DM9000_EPCR, 0x0);
}

static int dm9000_set_mac(struct net_device *ndev, const u8 *pMac)
{
	int i;

	for (i = 0; i < MAC_ADR_LEN; i++)
	{
		dm9000_writeb(DM9000_PAR + i, ndev->mac_addr[i]);
	}

	return 0;
}

static int dm9000_reset(void)
{
	int i;
	u8 status;

	dm9000_writeb(DM9000_NCR, 1);
	while (dm9000_readb(DM9000_NCR) & 1);

	status = dm9000_readb(DM9000_ISR);
	dm9000_writeb(DM9000_ISR, status);

	dm9000_writeb(DM9000_TCR, 0);
	dm9000_writeb(DM9000_RCR, 0);
//	dm9000_writeb(DM9000_BPTR, 0x3f);
//	dm9000_writeb(DM9000_FCTR, 0x38);
	dm9000_writeb(DM9000_SMCR, 0);

	dm9000_writeb(DM9000_GPR, 0x00); //

	for (i = 0; i < 8; i++)
	{
		dm9000_writeb(DM9000_MAR + i, 0xff);
	}

	return 0;
}

static int dm9000_send_packet(struct net_device *ndev, struct sock_buff *skb)
{
	int i;
	const u16 *tx_data;
	u16 tx_size;
	u32 flag;

	lock_irq_psr(flag);

	writeb(VA(DM9000_INDEX_PORT), DM9000_MWCMD);

	tx_data = (const u16 *)skb->data;
	tx_size = skb->size;

	for (i = 0; i < tx_size; i += sizeof(*tx_data))
	{
		writew(VA(DM9000_DATA_PORT), *tx_data);
		tx_data++;
	}

	dm9000_writeb(DM9000_TXPLL, tx_size & 0xff);
	dm9000_writeb(DM9000_TXPLH, (tx_size >> 8) & 0xff);

	dm9000_writeb(DM9000_TCR, 1);

	while (dm9000_readb(DM9000_TCR) & 1);

	ndev->stat.tx_packets++;

	unlock_irq_psr(flag);

	return 0;
}

static int dm9000_recv_packet(struct net_device *ndev)
{
	int i;
	u8 val;
	u16 *rx_data;
	u16 rx_stat, rx_size;
	struct sock_buff *skb;

	while (1)
	{
		dm9000_readb(DM9000_MRCMDX);
		val = readb(VA(DM9000_DATA_PORT));

		if (val != 0x1)
		{
			if (0 == val)
				return 0;

			printf("\n%s(), line %d: wrong status = 0x%02x\n",
				__func__, __LINE__, val);

			dm9000_reset();
			dm9000_writeb(DM9000_IMR, IMR_VAL);
			dm9000_writeb(DM9000_RCR, 1);

			return -EIO;
		}

		writeb(VA(DM9000_INDEX_PORT), DM9000_MRCMD);
		rx_stat = readw(VA(DM9000_DATA_PORT));
		rx_size = readw(VA(DM9000_DATA_PORT));

		// TODO: check the size
		DPRINT("stat = 0x%04x, size = 0x%04x\n", rx_stat, rx_size);

		if ((rx_stat & 0xbf00) || (skb = skb_alloc(0, (rx_size + 1) & ~1)) == NULL)
		{
			for (i = 0; i < rx_size; i += 2)
			{
				readw(VA(DM9000_DATA_PORT));
			}

			printf("\n%s(), line %d error: status = 0x%04x, size = %d\n",
			 		__func__, __LINE__, rx_stat, rx_size);
		}
		else
		{
			rx_data  = (u16 *)skb->data;  // or head;

			for (i = 0; i < rx_size; i += 2)
			{
				*rx_data = readw(VA(DM9000_DATA_PORT));
				DPRINT("%04x", *rx_data);
				rx_data++;
			}

			skb->size -= 4;

			netif_rx(skb);

			ndev->stat.rx_packets++;
		}
	}

	return 0;
}

static int dm9000_isr(u32 irq, void *dev)
{
	u8 status;
	struct net_device* ndev = dev;

	status = dm9000_readb(DM9000_ISR);

	if (0 == status)
		return 0;

	dm9000_writeb(DM9000_ISR, status);

	DPRINT("%s() line %d: status = 0x%08x\n",
		__func__, __LINE__, status);

	if (status & 0x1)
	{
		status = dm9000_readb(DM9000_RSR);
		if (status & 0xBF)
		{
			printf("%s(): RX Status = 0x%02x\n", __func__, status);
		}

		dm9000_recv_packet(ndev);
	}

	//fixme: move to upper layer
	if (status & 0x20)
	{
		u8 link = dm9000_readb(0x1) & 1 << 6;

		printf("dm9000 link %s\n", link ? "up" : "down");
	}

	return 0;
}

#ifndef CONFIG_IRQ_SUPPORT
static int dm9000_poll(struct net_device *ndev)
{
	return dm9000_isr(CONFIG_DM9000_IRQ, ndev);
}
#endif

static int __INIT__ dm9000_init(void)
{
	int ret;
	u16 ven_id, dev_id;
	u8  rev;
	struct net_device *ndev;
	const char *chip_name;

	ven_id = (dm9000_readb(DM9000_VIDH) << 8) | dm9000_readb(DM9000_VIDL);
	dev_id = (dm9000_readb(DM9000_PIDH) << 8) | dm9000_readb(DM9000_PIDL);

	if (ven_id != VENDOR_ID_DAVICOM)
	{
		printf("No DM9000X found! (ID = %04x:%04x)\n", ven_id, dev_id);
		return -ENODEV;
	}

	rev = dm9000_readb(DM9000_REV);
	// fixme: "DM9000B"
	switch (rev)
	{
	case 0x18:
	case 0x19:
		chip_name = "DM9000A";
		break;

	case 0x1A:
		chip_name = "DM9000C";
		break;

	default:
		chip_name = "DM9000E";
		break;
	}

	printf("%s: Vendor ID = 0x%04x, Product ID = 0x%04x, Rev = 0x%02x.\n",
		chip_name, ven_id, dev_id, rev);

	dm9000_reset();

	ndev = ndev_new(0);
	if (NULL == ndev)
		return -ENOMEM;

	ndev->chip_name    = chip_name;
	//
	ndev->send_packet  = dm9000_send_packet;
	ndev->set_mac_addr = dm9000_set_mac;
#ifndef CONFIG_IRQ_SUPPORT
	ndev->ndev_poll    = dm9000_poll;
#endif
	// MII
	ndev->phy_mask   = 2;
	ndev->mdio_read  = dm9000_mdio_read;
	ndev->mdio_write = dm9000_mdio_write;

	ret = irq_register_isr(CONFIG_DM9000_IRQ, dm9000_isr, ndev);

	ret = ndev_register(ndev);

	// TODO:  try to manually update when RX/TX done
	dm9000_reset();
	dm9000_writeb(DM9000_IMR, IMR_VAL);
	dm9000_writeb(DM9000_RCR, 1);

	return ret;
}

DRIVER_INIT(dm9000_init);
