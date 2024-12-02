/*
 * SPDX-License-Identifier:    GPL-2.0+
 *
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright (C) 2018-2020 MicroSys Electronics GmbH
 *
 */

#include <common.h>
#include <fsl_ddr_sdram.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int fsl_initdram(void)
{
    gd->ram_size = tfa_get_dram_size();

    if (!gd->ram_size)
        gd->ram_size = fsl_ddr_sdram_size();

    return 0;
}
