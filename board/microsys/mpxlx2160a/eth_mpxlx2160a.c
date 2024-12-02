// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright (C) 2020 MicroSys Electronics GmbH
 *
 */

#include <common.h>
#include <command.h>
#include <fdt_support.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>
#include "../../freescale/common/qsfp_eeprom.h"
#include "../crx06/mmd.h"

DECLARE_GLOBAL_DATA_PTR;

int board_eth_init(struct bd_info *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
	struct memac_mdio_info mdio_info;
	struct memac_mdio_controller *reg;
	int i, interface;
	struct mii_dev *dev;
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;

	/* Register the EMI 1 */
	fm_memac_mdio_init(bis, &mdio_info);

	reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO2;
	mdio_info.regs = reg;
	mdio_info.name = DEFAULT_WRIOP_MDIO2_NAME;

	/* Register the EMI 2 */
	fm_memac_mdio_init(bis, &mdio_info);

	switch (srds_s1) {
	case 19:
#if defined(CONFIG_CARRIER_CRX08)
		wriop_set_phy_address(WRIOP1_DPMAC3, 0,
				      USXGMII_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC4, 0,
				      USXGMII_PHY_ADDR2);

		wriop_set_phy_address(WRIOP1_DPMAC17, 0,
				      RGMII_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC18, 0,
				      RGMII_PHY_ADDR2);
		wriop_init_dpmac_enet_if(WRIOP1_DPMAC5,
					 PHY_INTERFACE_MODE_25G_AUI);
		wriop_init_dpmac_enet_if(WRIOP1_DPMAC6,
					 PHY_INTERFACE_MODE_25G_AUI);
#endif
		break;

	default:
		printf("SerDes1 protocol %d is not supported on LX2160ARDB\n",
		       srds_s1);
		goto next;
	}

	/* Setup the DPMACs to the desired MDIO1/2 */
	for (i = WRIOP1_DPMAC1; i <= WRIOP1_DPMAC10; i++) {
		interface = wriop_get_enet_if(i);
		if (interface != PHY_INTERFACE_MODE_NONE)
			printf(" DPMAC%d@%s",i,phy_string_for_interface(interface));
		switch (interface) {
#if defined(CONFIG_CARRIER_CRX08)
		case PHY_INTERFACE_MODE_XFI:
		case PHY_INTERFACE_MODE_XGMII:
		case PHY_INTERFACE_MODE_USXGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
			mv88x3310_init(dev, wriop_get_phy_address(i, 0), MDIO_CLAUSE_45);
			wriop_set_mdio(i, dev);
			break;
#endif
		default:
			break;
		}
	}

	/* Setup the 2 RGMII Interfaces */
	for (i = WRIOP1_DPMAC17; i <= WRIOP1_DPMAC18; i++) {
		interface = wriop_get_enet_if(i);
		if (interface != PHY_INTERFACE_MODE_NONE)
			printf(" DPMAC%d@%s",i,phy_string_for_interface(interface));
		switch (interface) {
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_ID:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
			wriop_set_mdio(i, dev);
			break;
        default:
			break;
		}
	}
	puts("\n");

next:
	cpu_eth_init(bis);
#endif /* CONFIG_FSL_MC_ENET */

	return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
#if defined(CONFIG_FSL_MC_ENET)
	mc_env_boot();
#endif
}
#endif /* CONFIG_RESET_PHY_R */

int fdt_fixup_board_phy(void *fdt)
{
	return 0;
}
