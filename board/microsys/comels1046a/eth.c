/*
 * SPDX-License-Identifier:    GPL-2.0+
 *
 * Copyright (C) 2017-2020 MicroSys Electronics GmbH
 *
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fsl_memac.h>
#include <fsl_tgec.h>
#include <spi.h>

#include "../../freescale/common/fman.h"

static int configure_phys(void)
{
    return 0;
}

int board_eth_init(struct bd_info *bis)
{
    int i;

    putc('\n');

#ifdef CONFIG_FMAN_ENET

    struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
    u32 srds_s1 =
        in_be32(&gur->rcwsr[4]) & FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
    srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

#define NUM_DTSEC_MDIO 1

    struct memac_mdio_info dtsec_mdio_info[NUM_DTSEC_MDIO] = {
        {.regs = (struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR,
         .name = DEFAULT_FM_MDIO_NAME},
    };

    struct mii_dev *dev;

    /* Register the 1G MDIO bus */
    for (i = 0; i < NUM_DTSEC_MDIO; i++) {
        fm_memac_mdio_init(bis, &dtsec_mdio_info[i]);
    }

    fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY1_ADDR);
    fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY2_ADDR);
    fm_info_set_phy_address(FM1_DTSEC9, SGMII_PHY3_ADDR);

    dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);

    fm_info_set_mdio(FM1_DTSEC3, dev);
    fm_info_set_mdio(FM1_DTSEC4, dev);
    fm_info_set_mdio(FM1_DTSEC9, dev);

    fm_disable_port(FM1_DTSEC5);
    fm_disable_port(FM1_DTSEC6);
    fm_disable_port(FM1_DTSEC10);

    configure_phys();
    cpu_eth_init(bis);

#endif

    return pci_eth_init(bis);
}
