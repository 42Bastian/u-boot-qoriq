/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/fsl_serdes.h>
#include <asm/immap_85xx.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fsl_dtsec.h>

#include "fman.h"
#include "../../../drivers/net/fm/fm.h"

/* TBD. Need to define this function for T104x, as it is missing in the default sources */
void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

//	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

/*
 * configure LEDs on 88E1512
 */

static int configure_leds(void)
{
	struct mii_dev *bus = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
	int i;

	if (!bus)
		return -1;

	for (i=0; i<4; i++)
	{
		/* Select page 3 */
		bus->write(bus, i, MDIO_DEVAD_NONE, 22, 3);
		/* invert LED polarity */
		bus->write(bus, i, MDIO_DEVAD_NONE, 17, 0x4415);
		if ((i == 1) || (i == 3))
		      bus->write(bus, i, MDIO_DEVAD_NONE, 16, 0x1012);
		/* Select page 0 */
		bus->write(bus, i, MDIO_DEVAD_NONE, 22, 0);
	}
	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct memac_mdio_info memac_mdio_info;
	unsigned int i;
	int phy_addr = 0;

	printf("Initializing Fman\n");

	memac_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;
	memac_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the real 1G MDIO bus */
	fm_memac_mdio_init(bis, &memac_mdio_info);

	fm_info_set_mdio(FM1_DTSEC5, NULL);
	fm_disable_port(FM1_DTSEC5);

	/*
	 * Program on board RGMII, SGMII PHY addresses.
	 */
	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1;

		switch (fm_info_get_enet_if(i)) {

		case PHY_INTERFACE_MODE_SGMII:
			if (FM1_DTSEC1 == i)
				phy_addr = CONFIG_SYS_SGMII1_PHY_ADDR;
			if (FM1_DTSEC2 == i)
				phy_addr = CONFIG_SYS_SGMII2_PHY_ADDR;
			if (FM1_DTSEC3 == i)
				phy_addr = CONFIG_SYS_SGMII3_PHY_ADDR;
			fm_info_set_phy_address(i, phy_addr);
			break;

		case PHY_INTERFACE_MODE_RGMII:
			if (FM1_DTSEC4 == i)
				phy_addr = CONFIG_SYS_RGMII1_PHY_ADDR;
			fm_info_set_phy_address(i, phy_addr);
			break;
		case PHY_INTERFACE_MODE_QSGMII:
			fm_info_set_phy_address(i, 0);
			break;
		case PHY_INTERFACE_MODE_NONE:
			fm_info_set_phy_address(i, 0);
			break;
		default:
			printf("Fman1: DTSEC%u set to unknown interface %i\n",
			       idx + 1, fm_info_get_enet_if(i));
			fm_info_set_phy_address(i, 0);
			break;
		}
		if (fm_info_get_enet_if(i) == PHY_INTERFACE_MODE_QSGMII ||
		    fm_info_get_enet_if(i) == PHY_INTERFACE_MODE_NONE)
			fm_info_set_mdio(i, NULL);
		else
			fm_info_set_mdio(i,
					 miiphy_get_dev_by_name(
							DEFAULT_FM_MDIO_NAME));
	}


	cpu_eth_init(bis);
#endif
	configure_leds();
	return pci_eth_init(bis);
}
