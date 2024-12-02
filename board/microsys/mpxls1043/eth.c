/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright (C) 2018-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fsl_dtsec.h>

#include "../../freescale/common/fman.h"
#include "../common/eth_common.h"

int board_eth_init(struct bd_info *bis)
{
#ifdef CONFIG_FMAN_ENET
    struct memac_mdio_info dtsec_mdio_info;
    struct mii_dev *dev;
    u32 srds_s1;
    struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

    srds_s1 = in_be32(&gur->rcwsr[4]) & FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
    srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

    dtsec_mdio_info.regs =
        (struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;

    dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

    /* Register the 1G MDIO bus */
    fm_memac_mdio_init(bis, &dtsec_mdio_info);

    /* Set the two on-board SGMII PHY address */
    fm_info_set_phy_address(FM1_DTSEC9, SGMII_PHY1_ADDR);
    fm_info_set_phy_address(FM1_DTSEC2, SGMII_PHY2_ADDR);

    fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY3_ADDR);
#if 0
    switch (srds_s1) {
    case 0x3355:
        break;
    default:
        printf("Invalid SerDes protocol 0x%x for MPXLS1043\n", srds_s1);
        break;
    }
#endif
    dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);

    fm_info_set_mdio(FM1_DTSEC2, dev);
    fm_info_set_mdio(FM1_DTSEC9, dev);

    fm_info_set_mdio(FM1_DTSEC3, dev);

    fm_info_set_mdio(FM1_DTSEC4, NULL);
    fm_disable_port(FM1_DTSEC4);

    cpu_eth_init(bis);
#endif
    configure_phys();
    return pci_eth_init(bis);
}
