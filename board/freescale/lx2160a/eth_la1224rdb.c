// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 *
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <malloc.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <exports.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>
#include "../common/qsfp_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

int board_eth_init(bd_t *bis)
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

	switch (srds_s1) {
	case 18:
	case 22:
		wriop_set_phy_address(WRIOP1_DPMAC3, 0,
				      AQR113_PHY_ADDR1);
		wriop_set_phy_address(WRIOP1_DPMAC4, 0,
				      AQR113_PHY_ADDR2);
		wriop_set_phy_address(WRIOP1_DPMAC17, 0,
				      RGMII_PHY_ADDR1);
		break;

	default:
		printf("SerDes1 protocol 0x%x is not supported on LA1224RDB\n",
		       srds_s1);
		goto next;
	}

	for (i = WRIOP1_DPMAC3; i <= WRIOP1_DPMAC5; i++) {
		interface = wriop_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_XGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
			wriop_set_mdio(i, dev);
			break;
		case PHY_INTERFACE_MODE_25G_AUI:
			/* TO DO for TI retimer DS250DF230 */
			break;
		default:
			break;
		}
	}

		interface = wriop_get_enet_if(WRIOP1_DPMAC17);
		switch (interface) {
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_ID:
			dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
			wriop_set_mdio(WRIOP1_DPMAC17, dev);
			break;
		default:
			break;
		}

next:
	cpu_eth_init(bis);
#endif /* CONFIG_FSL_MC_ENET */

#ifdef CONFIG_PHY_AQUANTIA
	/*
	 * Export functions to be used by AQ firmware
	 * upload application
	 */
	gd->jt->strcpy = strcpy;
	gd->jt->mdelay = mdelay;
	gd->jt->mdio_get_current_dev = mdio_get_current_dev;
	gd->jt->phy_find_by_mask = phy_find_by_mask;
	gd->jt->mdio_phydev_for_ethname = mdio_phydev_for_ethname;
	gd->jt->miiphy_set_current_dev = miiphy_set_current_dev;
#endif
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
	/* TO DO */
	return 0;
}
