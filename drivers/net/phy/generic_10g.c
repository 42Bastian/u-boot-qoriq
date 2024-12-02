// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic PHY Management code
 *
 * Copyright 2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 *
 * Based loosely off of Linux's PHY Lib
 */
#include <common.h>
#include <miiphy.h>
#include <phy.h>

int gen10g_shutdown(struct phy_device *phydev)
{
	return 0;
}


static int gen10g_mmd_dev_read_link_status(struct phy_device *phydev, int devad, int offset)
{
	const int reg = phy_read(phydev, devad, MDIO_STAT1+offset);
	return (reg >= 0) && ((reg&BIT(2)) != 0);
}

static int gen10g_mmd_dev_startup(struct phy_device *phydev, int devad, int offset)
{
	int link;
	int ctl = 0;
	int count;

	if (SPEED_1000 == phydev->speed)
		ctl |= BMCR_SPEED1000;
	else if (SPEED_100 == phydev->speed)
		ctl |= BMCR_SPEED100;
	else if (SPEED_10000 == phydev->speed) {
		ctl |= (1<<6)|(1<<13);
		ctl &= ~(0xf<<2);
	}
	else if (SPEED_2500 == phydev->speed) {
		ctl |= (1<<6)|(1<<13);
		ctl &= ~(0xf<<2);
		ctl |= (0x6<<2);
	}
	else if (SPEED_5000 == phydev->speed) {
		ctl |= (1<<6)|(1<<13);
		ctl &= ~(0xf<<2);
		ctl |= (0x7<<2);
	}

	ctl |= BIT(15);

	phy_write(phydev, devad, MDIO_CTRL1+offset, ctl);

	count = 100;
	gen10g_mmd_dev_read_link_status(phydev, devad, offset);
	do {
		link = gen10g_mmd_dev_read_link_status(phydev, devad, offset);
		if (!link) udelay(500*1000);
	} while (  !link // link is up?
			&& (--count > 0)
	);

	return 0;
}

int gen10g_startup(struct phy_device *phydev)
{
	int devad, reg;
	u32 mmd_mask = phydev->mmds & MDIO_DEVS_LINK;

	phydev->link = 1;

	/* For now just lie and say it's 10G all the time */
	phydev->speed = SPEED_10000;
	phydev->duplex = DUPLEX_FULL;

	phydev->pause = phydev->asym_pause = 0;

	gen10g_mmd_dev_startup(phydev, MDIO_MMD_PMAPMD, 0);

#ifdef CONFIG_CARRIER_CRX06
	/* Only CRX06 rev1:
	 * The H unit of the PHYs is not connected rather than it
	 * is present. Skip it because it will never show up a link.
	 */
	mmd_mask &= ~BIT(MDIO_MMD_PHYXS);
#endif

	/*
	 * Go through all the link-reporting devices, and make sure
	 * they're all up and happy
	 */
	for (devad = 0; mmd_mask; devad++, mmd_mask = mmd_mask >> 1) {
		if (!(mmd_mask & 1))
			continue;

		/* Read twice because link state is latched and a
		 * read moves the current state into the register */
#if defined(CONFIG_CARRIER_CRX08) || defined(CONFIG_CARRIER_CRX06)
		if ((devad == 4) && (
			(phydev->interface == PHY_INTERFACE_MODE_USXGMII) ||
			(phydev->interface == PHY_INTERFACE_MODE_XGMII) ||
			(phydev->interface == PHY_INTERFACE_MODE_XFI) ))
		{
			// The PCS for 10GBASE-R (XFI/XGMII/USXGMII is + $1000)
			phy_read(phydev, devad, MDIO_STAT1+0x1000);
			reg = phy_read(phydev, devad, MDIO_STAT1+0x1000);
		} else
#endif
		{
			phy_read(phydev, devad, MDIO_STAT1);
			reg = phy_read(phydev, devad, MDIO_STAT1);
		}

		if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS))
			phydev->link = 0;
	}

	return 0;
}

int gen10g_discover_mmds(struct phy_device *phydev)
{
	int mmd, stat2, devs1, devs2;

	/* Assume PHY must have at least one of PMA/PMD, WIS, PCS, PHY
	 * XS or DTE XS; give up if none is present. */
	for (mmd = MDIO_MMD_PMAPMD; mmd <= MDIO_MMD_DTEXS; mmd++) {
		/* Is this MMD present? */
		stat2 = phy_read(phydev, mmd, MDIO_STAT2);
		if (stat2 < 0 ||
			(stat2 & MDIO_STAT2_DEVPRST) != MDIO_STAT2_DEVPRST_VAL)
			continue;

		/* It should tell us about all the other MMDs */
		devs1 = phy_read(phydev, mmd, MDIO_DEVS1);
		devs2 = phy_read(phydev, mmd, MDIO_DEVS2);
		if (devs1 < 0 || devs2 < 0)
			continue;

		phydev->mmds = devs1 | (devs2 << 16);
		return 0;
	}

	return 0;
}

int gen10g_config(struct phy_device *phydev)
{
	/* For now, assume 10000baseT. Fill in later */
	phydev->supported = phydev->advertising = SUPPORTED_10000baseT_Full;

	return gen10g_discover_mmds(phydev);
}

struct phy_driver gen10g_driver = {
	.uid		= 0xffffffff,
	.mask		= 0xffffffff,
	.name		= "Generic 10G PHY",
	.features	= 0,
	.config		= gen10g_config,
	.startup	= gen10g_startup,
	.shutdown	= gen10g_shutdown,
};
