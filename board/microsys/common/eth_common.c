/* -*-C-*- */
/*
 * Copyright (C) 2018-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include "eth_common.h"

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_dtsec.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fsl_memac.h>
#include <fsl_tgec.h>

#ifdef CONFIG_TARGET_MPXLS1088
#include <fsl-mc/fsl_mc.h>
#include <fsl-mc/ldpaa_wriop.h>
#endif

#include "../../freescale/common/fman.h"

int configure_phys(void)
{
#ifdef CONFIG_TARGET_MPXLS1088
    struct mii_dev *bus = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
#else
    struct mii_dev *bus = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
#endif

#define LED_SHIFT(N) ((N)*4)
#define LED_MASK(N)  (0xf << LED_SHIFT(N))
#define LED_BLINK(N) (0b0001 << LED_SHIFT(N))
#define LED_LINK(N)  (0b0110 << LED_SHIFT(N))
#define LED_ACT(N)   (0b0010 << LED_SHIFT(N))
#define LED_OFF(N)   (0b1000 << LED_SHIFT(N))
#define LED_ON(N)    (0b1001 << LED_SHIFT(N))

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

#ifdef CONFIG_CARRIER_CRX05
            if ((i % 2) == 0) {
                // LED[0]:
                reg &= ~LED_MASK(0);
                reg |= LED_BLINK(0);

                // LED[2]:
                reg &= ~LED_MASK(2);
                reg |= LED_LINK(2);
            } else {
                // LED[0]:
                reg &= ~LED_MASK(0);
                reg |= LED_ACT(0);

                // LED[2]:
                reg &= ~LED_MASK(2);
                reg |= LED_LINK(2);
            }
#else
            if ((i % 2) == 0) {
                // LED[0]:
                reg &= ~LED_MASK(0);
                reg |= LED_BLINK(0);

                // LED[1]:
                reg &= ~LED_MASK(1);
                reg |= LED_LINK(1);

            } else {
                // LED[0]:
                reg &= ~LED_MASK(0);
                reg |= LED_ACT(0);

                // LED[2]:
                reg &= ~LED_MASK(2);
                reg |= LED_LINK(2);
            }
#endif

            bus->write(bus, i, MDIO_DEVAD_NONE, 16, reg);

            /* Select page 0 */
            bus->write(bus, i, MDIO_DEVAD_NONE, 22, 0);
        }
    }
    return 0;
}

/******************************************************************************
 * Local Variables:
 * mode: C
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * kate: space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
 * vim: set expandtab filetype=c:
 * vi: set et tabstop=4 shiftwidth=4: */
