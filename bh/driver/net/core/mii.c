#include <net/net.h>
#include <net/mii.h>

int mii_get_link_speed(struct mii_phy *phy)
{
	int i;
	struct net_device *ndev = phy->ndev;

	for(i = 0; i < 40; i++)
	{
		if (ndev->mdio_read(ndev, phy->mii_id, MII_REG_BMS) & 0x4) // or 0x24
		{
#warning
#ifdef CONFIG_LAN9220
			__u16 link = ndev->mdio_read(ndev, phy->mii_id, MII_REG_SPCS);

			switch ((link >> 2) & 0x7)
			{
			case 1:
				return ETHER_SPEED_10M_HD;

			case 5:
				return ETHER_SPEED_10M_FD;

			case 2:
				return ETHER_SPEED_100M_HD;

			case 6:
				return ETHER_SPEED_100M_FD;

			default:
				printf("link status = 0x%04x\n", link);
				break;
			}
#else
			__u16 link = ndev->mdio_read(ndev, phy->mii_id, MII_REG_STAT);

			switch (link >> 12)
			{
			case 1:
				return ETHER_SPEED_10M_HD;

			case 2:
				return ETHER_SPEED_10M_FD;

			case 4:
				return ETHER_SPEED_100M_HD;

			case 8:
				return ETHER_SPEED_100M_FD;

			default:
				printf("link status = 0x%04x\n", link);
				break;
			}
#endif
		}

		mdelay(100);
	}

	return ETHER_SPEED_UNKNOW;
}

int mii_get_link_connection(struct mii_phy *phy)
{
	struct net_device *ndev = phy->ndev;

	return ndev->mdio_read(ndev, phy->mii_id, MII_REG_BMS) & 0x4;
}

struct mii_phy *mii_phy_probe(struct net_device *ndev, __u8 mii_id)
{
	__u16 ven_id, dev_id;
	struct mii_phy *phy;

	ven_id = ndev->mdio_read(ndev, mii_id, MII_REG_ID1);
	dev_id = ndev->mdio_read(ndev, mii_id, MII_REG_ID2);

	DPRINT("%s(): ID = 0x%04x, 0x%04x @ %d\n",
		__func__, ven_id, dev_id, mii_id);

	if (ven_id == 0xFFFF || ven_id == 0x0000 ||
			dev_id == 0xFFFF || dev_id == 0x0000)
		return NULL;

	phy = zalloc(sizeof(*phy));
	// if null

	phy->mii_id = mii_id;
	phy->ven_id = ven_id;
	phy->dev_id = dev_id;

	return phy;
}

void mii_reset_phy(struct net_device *ndev, struct mii_phy *phy)
{
	__u16 val;
	int time_out = 100;

	ndev->mdio_write(ndev, phy->mii_id, MII_REG_BMC, MII_PHY_RESET);

	while (time_out > 0)
	{
		val = ndev->mdio_read(ndev, phy->mii_id, MII_REG_BMC);
		if (!(val & MII_PHY_RESET))
			return;

		udelay(10);
		time_out--;
	}

	printf("PHY reset failed!\n");
}
