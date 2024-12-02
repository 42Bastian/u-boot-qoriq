/* -*-C-*- */
/*
 * Copyright (C) 2017-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef MMD_H
#define MMD_H

#include <common.h>
#include <phy.h>

#include <linux/mdio.h>

#define MV88X3310_DEVAD_C_UNIT MDIO_MMD_VEND2

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        MDIO_CLAUSE_22,
        MDIO_CLAUSE_45
    } mdio_protocol_t;

    typedef struct mmd_cntx_s {
        struct mii_dev *bus;
        int addr;
        mdio_protocol_t mdio_proto;
        uint16_t __cached_devad;
        uint16_t __cached_reg;
    } mmd_cntx_t;

#define INIT_MMD_CNTX(CNTX,BUS,ADDR) mmd_cntx_t CNTX = {.bus = BUS, .addr = ADDR,\
        .mdio_proto = MDIO_CLAUSE_22, .__cached_devad = ~0, .__cached_reg = ~0}

    extern int mmd_read(mmd_cntx_t * cntx, uint16_t devad, uint16_t reg);
    extern int mmd_write(mmd_cntx_t * cntx, uint16_t devad, uint16_t reg,
                         uint16_t val);

    extern int mv88x3310_init(struct mii_dev *bus, const int addr,
                              const mdio_protocol_t proto);

#ifdef __cplusplus
}                               /* extern "C" */
#endif
#endif                          /* MMD_H */

/* *INDENT-OFF* */
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
