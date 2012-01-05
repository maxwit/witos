#include <init.h>
#include <delay.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <net/net.h>
#include <net/mii.h>

#if 0
int mii_get_link_connection(struct mii_phy *phy)
{
	struct net_device *ndev = phy->ndev;

	return ndev->mdio_read(ndev, phy->addr, MII_REG_BMS) & 0x4;
}
#endif

struct mii_phy *mii_phy_probe(struct net_device *ndev, __u8 addr)
{
	__u16 ven_id, dev_id;
	struct mii_phy *phy;

	ven_id = ndev->mdio_read(ndev, addr, MII_REG_ID1);
	dev_id = ndev->mdio_read(ndev, addr, MII_REG_ID2);

	DPRINT("%s(): ID = 0x%04x, 0x%04x @ %d\n",
		__func__, ven_id, dev_id, addr);

	if (ven_id == 0x0 || dev_id == 0x0)
		return NULL;

	phy = zalloc(sizeof(*phy));
	// if null

	phy->addr = addr;
	phy->ven_id = ven_id;
	phy->dev_id = dev_id;

	return phy;
}

int mii_reset_phy(struct net_device *ndev, struct mii_phy *phy)
{
	__u16 val;
	int time_out = 100;

	ndev->mdio_write(ndev, phy->addr, MII_REG_BMC, MII_PHY_RESET);

	while (time_out > 0) {
		val = ndev->mdio_read(ndev, phy->addr, MII_REG_BMC);
		if (!(val & MII_PHY_RESET))
			return 0;

		udelay(10);
		time_out--;
	}

	printf("PHY reset failed!\n");
	return -ETIMEDOUT;
}
