/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright (C) 2018-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __DDR_H__
#define __DDR_H__

void erratum_a008850_post(void);

struct board_specific_parameters {
    u32 n_ranks;
    u32 datarate_mhz_high;
    u32 rank_gb;
    u32 clk_adjust;
    u32 wrlvl_start;
    u32 wrlvl_ctl_2;
    u32 wrlvl_ctl_3;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
static const struct board_specific_parameters udimm0[] = {
    /*
     * memory controller 0
     *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl
     * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3
     */
    {1, 2100, 0, 0x9, 0xA, 0x0A090906, 0x05050406,},
    {1, 1666, 0, 0x9, 0xA, 0x0A090906, 0x05050406,},
    {}
};

static const struct board_specific_parameters *udimms[] = {
    udimm0,
};

#endif
