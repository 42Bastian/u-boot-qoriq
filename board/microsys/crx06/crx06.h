/* -*-C-*- */
/*
 * Copyright (C) 2018-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef CRX06_H
#define CRX06_H

#ifdef CONFIG_FSL_MC_ENET
#include <fsl-mc/ldpaa_wriop.h>
#else
#include <fm_eth.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

    extern int board_crx06_fdt_fixup_phy(void *fdt);

#ifdef CONFIG_FSL_MC_ENET
    extern int crx06_init_xfi(const char *const dtsec_mdio_name,
                              const char *const tgec_mdio_name,
                              enum wriop_port port, const int addr);
#else
    extern int crx06_init_xfi(const char *const dtsec_mdio_name,
                              const char *const tgec_mdio_name,
                              enum fm_port port, const int addr);
#endif

#ifdef __cplusplus
}                               /* extern "C" */
#endif
#endif                          /* CRX06_H */

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
