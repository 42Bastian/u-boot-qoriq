/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright (C) 2019 MicroSys Electronics GmbH
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

#include "../common/eth_common.h"

#ifdef CONFIG_CARRIER_CRX06
#include "../crx06/mmd.h"
#include "../crx06/crx06.h"

#define DPMAC1_PHY_ADDR 0x10
#define DPMAC2_PHY_ADDR 0x11

#endif

DECLARE_GLOBAL_DATA_PTR;

#define HAS_EM(N, EC) ((((EC)>>(6+(N)-1))&1)==0)
#define GET_EC(N, EC) (((EC)>>(3*((N)-1)))&0x7)
#define HAS_EC(N, EC) (GET_EC(N, EC)==0)

int board_eth_init(struct bd_info *bis)
{
#if defined(CONFIG_FSL_MC_ENET)
    int i, interface;
    struct memac_mdio_info mdio_info;
    struct mii_dev *dev;
    struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
    struct memac_mdio_controller *reg;
    u32 srds_s1, cfg, ec;

    cfg =
        in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
        FSL_CHASSIS3_SRDS1_PRTCL_MASK;
    cfg >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;

    srds_s1 = serdes_get_number(FSL_SRDS_1, cfg);

    ec = gur_in32(&gur->rcwsr[FSL_CHASSIS3_EC1_REGSR]);

#ifdef CONFIG_CARRIER_CRX05
    if (HAS_EM(1, ec) || HAS_EM(2, ec)) {   // EM1 or EM2
        reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
        mdio_info.regs = reg;
        mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;
        fm_memac_mdio_init(bis, &mdio_info);
    }
#endif

#ifdef CONFIG_CARRIER_CRX06
    if (HAS_EM(1, ec)) {        // EM1
        reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO1;
        mdio_info.regs = reg;
        mdio_info.name = DEFAULT_WRIOP_MDIO1_NAME;
        /* Register the EMI 1 */
        fm_memac_mdio_init(bis, &mdio_info);
    }

    if (HAS_EM(2, ec)) {        // EM2
        reg = (struct memac_mdio_controller *)CONFIG_SYS_FSL_WRIOP1_MDIO2;
        mdio_info.regs = reg;
        mdio_info.name = DEFAULT_WRIOP_MDIO2_NAME;
        /* Register the EMI 2 */
        fm_memac_mdio_init(bis, &mdio_info);
    }
#endif

#ifdef CONFIG_CARRIER_CRX05

    wriop_set_phy_address(WRIOP1_DPMAC2, 0, 0x00);
    wriop_set_phy_address(WRIOP1_DPMAC3, 0, 0x01);
    wriop_set_phy_address(WRIOP1_DPMAC7, 0, 0x02);
    wriop_set_phy_address(WRIOP1_DPMAC4, 0, 0x03);

    wriop_disable_dpmac(WRIOP1_DPMAC1);
    wriop_disable_dpmac(WRIOP1_DPMAC5);

    //printf("\n");
    dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
    for (i = WRIOP1_DPMAC1; i <= NUM_WRIOP_PORTS; i++) {
        interface = wriop_get_enet_if(i);
        if (wriop_is_enabled_dpmac(i)) {
            //printf("DPMAC%d: IF-Mode %d\n", i, interface);
            wriop_set_mdio(i, dev);
        }
    }

#endif

#ifdef CONFIG_CARRIER_CRX06

    switch (srds_s1) {
    case 0x19:                 // SerDes1 == 1143
    case 0x1D:                 // SerDes1 == 1144

        /*
         * Setup the two XFI interfaces:
         */
        if (HAS_EM(2, ec)) {    // EM2
            wriop_set_phy_address(WRIOP1_DPMAC1, 0, DPMAC1_PHY_ADDR);
            wriop_set_phy_address(WRIOP1_DPMAC2, 0, DPMAC2_PHY_ADDR);
        } else {
            wriop_disable_dpmac(WRIOP1_DPMAC1);
            wriop_disable_dpmac(WRIOP1_DPMAC2);
        }

        /*
         * Setup 2xQSGMII:
         */

        /*
         * In case that EC1 and EC2 are enabled MAC4 and MAC5 are superseded
         * by RGMII. That means that we're loosing these two MACs for QSGMII.
         */

        /*
         * QSGMII1:
         */
        if (!HAS_EC(1, ec))
            wriop_set_phy_address(WRIOP1_DPMAC4, 0, 0x09);
        if (!HAS_EC(2, ec))
            wriop_set_phy_address(WRIOP1_DPMAC5, 0, 0x0a);

        if (srds_s1 == 0x1d)    /* 2xQSGMII */
            wriop_set_phy_address(WRIOP1_DPMAC6, 0, 0x0b);
        else                    /* 2xTSN => 1xSGMII3 */
            wriop_disable_dpmac(WRIOP1_DPMAC6);

        wriop_set_phy_address(WRIOP1_DPMAC3, 0, 0x08);

        /*
         * QSGMII2:
         */
        wriop_set_phy_address(WRIOP1_DPMAC7, 0, 0x0c);
        wriop_set_phy_address(WRIOP1_DPMAC8, 0, 0x0d);
        wriop_set_phy_address(WRIOP1_DPMAC9, 0, 0x0e);
        wriop_set_phy_address(WRIOP1_DPMAC10, 0, 0x0f);

        break;
    default:
        printf("SerDes1 protocol 0x%x is not supported on CRX06\n", srds_s1);
        break;
    }

    for (i = WRIOP1_DPMAC1; i <= NUM_WRIOP_PORTS; i++) {
        interface = wriop_get_enet_if(i);
        switch (interface) {
        case PHY_INTERFACE_MODE_QSGMII:
        case PHY_INTERFACE_MODE_SGMII:
            dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
            wriop_set_mdio(i, dev);
            break;
        case PHY_INTERFACE_MODE_XGMII:
            dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
            wriop_set_mdio(i, dev);
            break;
        default:
            break;
        }
    }

    if (HAS_EM(2, ec)) {        // EM2
        /*
         * XFI2:
         */
        crx06_init_xfi(DEFAULT_WRIOP_MDIO1_NAME, DEFAULT_WRIOP_MDIO2_NAME,
                       WRIOP1_DPMAC2, DPMAC2_PHY_ADDR);

        /*
         * XFI1:
         */
        crx06_init_xfi(DEFAULT_WRIOP_MDIO1_NAME, DEFAULT_WRIOP_MDIO2_NAME,
                       WRIOP1_DPMAC1, DPMAC1_PHY_ADDR);
    }
#endif

    configure_phys();
    cpu_eth_init(bis);

#endif /* CONFIG_FMAN_ENET */

    return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
    mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */
