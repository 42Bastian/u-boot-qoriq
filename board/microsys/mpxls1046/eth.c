/*
 * Copyright (C) 2017-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
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

#ifdef CONFIG_CARRIER_CRX06
#include "../crx06/mmd.h"
#endif

static int configure_phys(void)
{
    struct mii_dev *bus = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
    int i, reg;
    u32 phy_id;

    if (!bus)
        return -1;

    for (i = 0; i < 8; i++) {
        phy_id = ~0;
        get_phy_id(bus, i, MDIO_DEVAD_NONE, &phy_id);

        if ((phy_id & 0xfffffff0) == 0x01410dd0) {  // MV 88E1510/88E1512

            bus->write(bus, i, MDIO_DEVAD_NONE, 22, 0x00ff);
            bus->write(bus, i, MDIO_DEVAD_NONE, 17, 0x214B);
            bus->write(bus, i, MDIO_DEVAD_NONE, 16, 0x2144);
            bus->write(bus, i, MDIO_DEVAD_NONE, 17, 0x0C28);
            bus->write(bus, i, MDIO_DEVAD_NONE, 16, 0x2146);
            bus->write(bus, i, MDIO_DEVAD_NONE, 17, 0xB233);
            bus->write(bus, i, MDIO_DEVAD_NONE, 16, 0x214D);
            bus->write(bus, i, MDIO_DEVAD_NONE, 17, 0xCC0C);
            bus->write(bus, i, MDIO_DEVAD_NONE, 16, 0x2159);
            bus->write(bus, i, MDIO_DEVAD_NONE, 22, 0x0000);
            bus->write(bus, i, MDIO_DEVAD_NONE, 22, 18);

            /* Select page 3 */
            bus->write(bus, i, MDIO_DEVAD_NONE, 22, 3);
            /* invert LED polarity */
            bus->write(bus, i, MDIO_DEVAD_NONE, 17, 0x4415);

            reg = bus->read(bus, i, MDIO_DEVAD_NONE, 16);

            if ((i % 2) == 0) {
                // LED[0]:
                reg &= ~0xf;
                reg |= 0b0001;

                // LED[1]:
                reg &= ~(0xf << 4);
                reg |= (0b0110 << 4);
            } else {
                // LED[0]:
                reg &= ~0xf;
                reg |= 0b0010;

                // LED[2]:
                reg &= ~(0xf << 8);
                reg |= (0b0110 << 8);
            }

            bus->write(bus, i, MDIO_DEVAD_NONE, 16, reg);

            /* Select page 0 */
            bus->write(bus, i, MDIO_DEVAD_NONE, 22, 0);
        }
    }
    return 0;
}

#if 0
int get_phy_id(struct mii_dev *bus, int addr, int devad, u32 * phy_id)
{
    int phy_reg;
#if defined(CONFIG_MPX1046_RCW_SDCARD_2XXFI)
    INIT_MMD_CNTX(cntx, bus, addr);
#endif

    //    printf("get_phy_id(%s, 0x%02x, %d)\n", bus->name, addr, devad);

#if defined(CONFIG_MPX1046_RCW_SDCARD_2XXFI)
    /*
     * Check for 10Gb PHYs:
     */
    if (strncmp(bus->name, DEFAULT_FM_MDIO_NAME, MDIO_NAME_LEN) == 0
        && (addr == 0x10 || addr == 0x11)) {

        phy_reg = mmd_read(&cntx, MDIO_MMD_PMAPMD, MII_PHYSID1);

        if (phy_reg < 0)
            return -EIO;

        *phy_id = (phy_reg & 0xffff) << 16;

        phy_reg = mmd_read(&cntx, MDIO_MMD_PMAPMD, MII_PHYSID2);

        if (phy_reg < 0)
            return -EIO;

        *phy_id |= (phy_reg & 0xffff);

        return 0;
    }
#endif

    /* Grab the bits from PHYIR1, and put them
     * in the upper half */
    phy_reg = bus->read(bus, addr, devad, MII_PHYSID1);

    if (phy_reg < 0)
        return -EIO;

    *phy_id = (phy_reg & 0xffff) << 16;

    /* Grab the bits from PHYIR2, and put them in the lower half */
    phy_reg = bus->read(bus, addr, devad, MII_PHYSID2);

    if (phy_reg < 0)
        return -EIO;

    *phy_id |= (phy_reg & 0xffff);

    return 0;
}
#endif

#if 0
int board_phy_config(struct phy_device *phydev)
{
    if (phydev->drv->config) {
        //        printf("%s PHY@%02x: 0x%08x %s\n",
        //                phydev->bus->name,
        //                phydev->addr,
        //                phydev->phy_id,
        //                phydev->drv->name);
        return phydev->drv->config(phydev);
    }
    return 0;
}
#endif

#ifdef CONFIG_CARRIER_CRX06
static int crx06_init_xfi(const char *const dtsec_mdio_name,
                          const char *const tgec_mdio_name, enum fm_port port,
                          const int addr)
{
    struct mii_dev *mdio;
    int ret = -1;

    if (dtsec_mdio_name) {
        mdio = miiphy_get_dev_by_name(dtsec_mdio_name);
        ret = mv88x3310_init(mdio, addr, MDIO_CLAUSE_22);
    }

    if (ret < 0 && tgec_mdio_name) {
        mdio = miiphy_get_dev_by_name(tgec_mdio_name);
        ret = mv88x3310_init(mdio, addr, MDIO_CLAUSE_45);
    }

    if (ret == 0) {
        fm_info_set_phy_address(port, addr);
        fm_info_set_mdio(port, mdio);
    } else
        fm_disable_port(port);

    return ret;
}
#endif

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

#define NUM_TGEC_MDIO 1

    struct memac_mdio_info tgec_mdio_info[NUM_TGEC_MDIO] = {
        {.regs = (struct memac_mdio_controller *)CONFIG_SYS_FM1_TGEC_MDIO_ADDR,
         .name = DEFAULT_FM_TGEC_MDIO_NAME},
    };

    struct mii_dev *dev;

    /* Register the 1G MDIO bus */
    for (i = 0; i < NUM_DTSEC_MDIO; i++) {
        fm_memac_mdio_init(bis, &dtsec_mdio_info[i]);
    }

    /* Register the 10G MDIO bus */
    for (i = 0; i < NUM_TGEC_MDIO; i++) {
        fm_memac_mdio_init(bis, &tgec_mdio_info[i]);
    }

#ifdef CONFIG_CARRIER_CRX06

#if 0
    if (srds_s1 == 0x3333) {

        /*
         * Note that this configuration is obsoleted!
         * It applies only for CRX06 rev1.
         */

        // 4xSGMII enabled at the CPU

        /*
         * RCW[128-143] == 3333
         *
         * SerDes1:
         * A SD1-3: SGMII.6  FM1@DTSEC6   PHY08-0B
         * B SD1-2: SGMII.5  FM1@DTSEC5   PHY0C-0F
         * C SD1-1: SGMII.10 FM1@DTSEC10
         * D SD1-0: SGMII.9  FM1@DTSEC9
         */

        /*
         * Disable SGMII.9 and SGMII.10, because they are routed to
         * the two 10Gb PHYs:
         */
        fm_disable_port(FM1_DTSEC10);   // XFI 10G
        fm_disable_port(FM1_DTSEC9);    // XFI 10G

        fm_info_set_phy_address(FM1_DTSEC5, 0xc);
        fm_info_set_phy_address(FM1_DTSEC6, 0x8);

        dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);

        fm_info_set_mdio(FM1_DTSEC5, dev);
        fm_info_set_mdio(FM1_DTSEC6, dev);
    }
#endif

    if (srds_s1 == 0x1133) {

        /*
         * RCW[128-143] == 1133
         */

        dev = miiphy_get_dev_by_name(dtsec_mdio_info[0].name);

        fm_info_set_phy_address(FM1_DTSEC5, 0xc);
        fm_info_set_phy_address(FM1_DTSEC6, 0x8);

        fm_info_set_mdio(FM1_DTSEC5, dev);
        fm_info_set_mdio(FM1_DTSEC6, dev);

        /*
         * XFI.10:
         */
        crx06_init_xfi(dtsec_mdio_info[0].name, tgec_mdio_info[0].name,
                       FM1_10GEC2, 0x10);

        /*
         * XFI.9:
         */
        crx06_init_xfi(dtsec_mdio_info[0].name, tgec_mdio_info[0].name,
                       FM1_10GEC1, 0x11);

    }

    if (srds_s1 == 0x1040) {

        dev = miiphy_get_dev_by_name(dtsec_mdio_info[0].name);

        fm_info_set_phy_address(FM1_DTSEC1, 0xf);
        fm_info_set_phy_address(FM1_DTSEC5, 0xd);
        fm_info_set_phy_address(FM1_DTSEC6, 0xc);
        fm_info_set_phy_address(FM1_DTSEC10, 0xe);

        fm_info_set_mdio(FM1_DTSEC1, dev);
        fm_info_set_mdio(FM1_DTSEC5, dev);
        fm_info_set_mdio(FM1_DTSEC6, dev);
        fm_info_set_mdio(FM1_DTSEC10, dev);

        /*
         * XFI.9:
         */
        crx06_init_xfi(dtsec_mdio_info[0].name, tgec_mdio_info[0].name,
                       FM1_10GEC1, 0x11);
    }
#else /* CONFIG_CARRIER_CRX05 is supposed to be the default */

    /* Set the on-board PHY address */
    fm_info_set_phy_address(FM1_DTSEC9, SGMII_PHY1_ADDR);
    fm_info_set_phy_address(FM1_DTSEC6, SGMII_PHY2_ADDR);
    fm_info_set_phy_address(FM1_DTSEC5, SGMII_PHY3_ADDR);
    fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY1_ADDR);

    dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);

    fm_info_set_mdio(FM1_DTSEC5, dev);
    fm_info_set_mdio(FM1_DTSEC6, dev);
    fm_info_set_mdio(FM1_DTSEC9, dev);
    fm_info_set_mdio(FM1_DTSEC3, dev);

    fm_info_set_mdio(FM1_DTSEC10, NULL);

    fm_disable_port(FM1_DTSEC10);

    fm_info_set_mdio(FM1_DTSEC4, NULL);

    fm_disable_port(FM1_DTSEC4);

#endif

    configure_phys();
    cpu_eth_init(bis);

#endif

    return pci_eth_init(bis);
}
