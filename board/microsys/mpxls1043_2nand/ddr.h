/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright (C) 2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __DDR_H__
#define __DDR_H__

extern void erratum_a008850_post(void);

struct board_specific_parameters {
    u32 n_ranks;
    u32 datarate_mhz_high;
    u32 rank_gb;
    u32 clk_adjust;
    u32 wrlvl_start;
    u32 wrlvl_ctl_2;
    u32 wrlvl_ctl_3;
    u32 cpo_override;
    u32 write_data_delay;
    u32 force_2t;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
static const struct board_specific_parameters udimm0[] = {
    /*
     * memory controller 0
     *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
     * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
     */
#ifdef CONFIG_SYS_FSL_DDR4
    /*{1,  1666, 0, 4,     1, 0x02020303, 0x00000003,}, */// nEval Board
    {1, 1600, 0, 9, 5, 0x07070800, 0x00000008,},
#endif
    {}
};

static const struct board_specific_parameters *udimms[] = {
    udimm0,
};

#endif
