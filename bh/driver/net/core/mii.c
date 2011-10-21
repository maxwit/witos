#include <net/net.h>
#include <net/mii.h>

int mii_get_link_speed(struct mii_phy *phy)
{
	int i;
	struct net_device *ndev = phy->ndev;

	for(i = 0; i < 40; i++)
	{
		if (ndev->mdio_read(ndev, phy->mii_id, MII_REG_BMS) & 0x20) // or 0x24
		{
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
	struct mii_phy *phy = zalloc(sizeof(*phy)); // fixme
	// if null

	phy->ven_id = ndev->mdio_read(ndev, mii_id, MII_REG_ID1);
	phy->dev_id = ndev->mdio_read(ndev, mii_id, MII_REG_ID2);

	DPRINT("%s(): ID = 0x%04x, 0x%04x @ %d\n",
		__func__, phy->ven_id, phy->dev_id, mii_id);

	if (phy->ven_id != 0xFFFF && phy->ven_id != 0x0000 &&
		phy->dev_id != 0xFFFF && phy->dev_id != 0x0000)
	{
		//
		phy->mii_id = mii_id;
		return phy;
	}

	free(phy);
	return NULL;
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
