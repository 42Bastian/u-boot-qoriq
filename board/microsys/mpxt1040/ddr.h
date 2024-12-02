/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Copyright 2015 MicroSys GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DDR_H__
#define __DDR_H__

#ifdef CONFIG_SYS_DDR_RAW_TIMING
#ifdef CONFIG_SYS_FSL_DDR4
/* 4GB discrete DDR4 K4A4G165WD-BCRC*/
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
#ifdef	CONFIG_DDR_4GB
	.rank_density = 4294967296u,
	.capacity = 4294967296u,
#else
	.rank_density = 2147483648u,
	.capacity = 2147483648u,
#endif
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
#ifdef	CONFIG_DDR_4GB
	.n_row_addr = 16,
#else
	.n_row_addr = 15,
#endif
	.n_col_addr = 10,
	.bank_addr_bits = 2,
	.bank_group_bits = 1,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 833,
	.tckmax_ps = 1600,
	.caslat_x = 0x000DFA00,
	.taa_ps = 14160,
	.trcd_ps = 14160,
	.trp_ps = 14160,
	.tras_ps = 32000,
	.trc_ps = 46160,
	.trfc1_ps = 260000,
	.trfc2_ps = 160000,
	.trfc4_ps = 110000,
	.tfaw_ps = 30000,
	.trrds_ps = 8400,
	.trrdl_ps = 9600,
	.tccdl_ps = 7202,
	.refresh_rate_ps = 3750000,
	.dq_mapping[0] = 0x0,
	.dq_mapping[1] = 0x0,
	.dq_mapping[2] = 0x0,
	.dq_mapping[3] = 0x0,
	.dq_mapping[4] = 0x0,
	.dq_mapping[5] = 0x0,
	.dq_mapping[6] = 0x0,
	.dq_mapping[7] = 0x0,
	.dq_mapping[8] = 0x0,
	.dq_mapping[9] = 0x0,
	.dq_mapping[10] = 0x0,
	.dq_mapping[11] = 0x0,
	.dq_mapping[12] = 0x0,
	.dq_mapping[13] = 0x0,
	.dq_mapping[14] = 0x0,
	.dq_mapping[15] = 0x0,
	.dq_mapping[16] = 0x0,
	.dq_mapping[17] = 0x0,
	.dq_mapping_ors = 0,
};
#else
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 1073741824u,
	.capacity = 1073741824u,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 14,
	.n_col_addr = 10,
	.n_banks_per_sdram_device = 8,
	.edc_config = 2,	/* ECC */
	.burst_lengths_bitmask = 0x0c,
	.tckmin_x_ps = 1071,
	.caslat_x = 0x2fe << 4,	/* 5,6,7,8,9,10,11,13 */
	.taa_ps = 13910,
	.twr_ps = 15000,
	.trcd_ps = 13910,
	.trrd_ps = 6000,
	.trp_ps = 13910,
	.tras_ps = 34000,
	.trc_ps = 47910,
	.trfc_ps = 160000,
	.twtr_ps = 7500,
	.trtp_ps = 7500,
	.refresh_rate_ps = 3900000,
	.tfaw_ps = 40000,
};
#endif
#endif

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
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl
	 * ranks| mhz| GB  |adjst| start |   ctl2
	 */
#ifdef CONFIG_SYS_FSL_DDR4
	{1,  1600, 2, 4,     4, 0x04030302, 0x02010102},
#elif defined(CONFIG_SYS_FSL_DDR3)
	{1,  833,  4, 4,     6, 0x06060607, 0x08080807},
	{1,  833,  0, 4,     6, 0x06060607, 0x08080807},
	{1,  1350, 4, 4,     7, 0x0708080A, 0x0A0B0C09},
	{1,  1350, 0, 4,     7, 0x0708080A, 0x0A0B0C09},
	{1,  1666, 4, 4,     7, 0x0808090B, 0x0C0D0E0A},
	{1,  1666, 0, 4,     7, 0x0808090B, 0x0C0D0E0A},
	{1,  833,  4, 4,     6, 0x06060607, 0x08080807},
	{1,  833,  0, 4,     6, 0x06060607, 0x08080807},
	{1,  1350, 4, 4,     7, 0x0708080A, 0x0A0B0C09},
	{1,  1350, 0, 4,     7, 0x0708080A, 0x0A0B0C09},
	{1,  1666, 4, 4,     7, 0x0808090B, 0x0C0D0E0A},
	{1,  1666, 0, 4,     7, 0x0808090B, 0x0C0D0E0A},
#else
#error DDR type not defined
#endif
	{}
};

#endif

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};
