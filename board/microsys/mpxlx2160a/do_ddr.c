// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Kei Thomsen, MicroSys GmbH, thomsen@microsys.de
 */

/*
 * Command for reading the 1D and 2D Training information from the DDR Phy
 * on LX2160.
 *
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/global_data.h>
#include "ddr-phy.h"

#define NXP_DDR_PHY1_ADDR          0x01400000
#define NXP_DDR_PHY2_ADDR          0x01600000
/* CSR Register (taken from ATF csr.h) */
#define csr_micro_cont_mux_sel_addr		0x00
#define csr_ucclk_hclk_enables_addr		0x80
#define t_apbonly				0xd0000
#define t_drtub					0xc0000

#define out16(a, v)        (*(volatile unsigned short *)(a) = (v))
#define in16(a)            (*(volatile unsigned short *)(a))

#define MAP_PHY_ADDR(pstate, n, instance, offset, c) \
		((((pstate * n) + instance + c) << 12) + offset)

/* Access to CSR Register of the DDR Phy Controller */

static uint32_t map_phy_addr_space(uint32_t addr)
{
	/* 23 bit addressing */
	int pstate =     (addr & 0x700000) >> 20; /* bit 22:20 */
	int block_type = (addr & 0x0f0000) >> 16; /* bit 19:16 */
	int instance =   (addr & 0x00f000) >> 12; /* bit 15:12 */
	int offset =     (addr & 0x000fff);       /* bit 11:0 */

	switch (block_type) {
	case 0x0: /* 0x0 : ANIB */
		return MAP_PHY_ADDR(pstate, 12, instance, offset, 0);
	case 0x1: /* 0x1 : DBYTE */
		return MAP_PHY_ADDR(pstate, 10, instance, offset, 0x30);
	case 0x2: /* 0x2 : MASTER */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x58);
	case 0x4: /* 0x4 : ACSM */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x5c);
	case 0x5: /* 0x5 : μCTL Memory */
		return MAP_PHY_ADDR(pstate, 0, instance, offset, 0x60);
	case 0x7: /* 0x7 : PPGC */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x68);
	case 0x9: /* 0x9 : INITENG */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x69);
	case 0xc: /* 0xC : DRTUB */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x6d);
	case 0xd: /* 0xD : APB Only */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x6e);
	default:
		printf("ERR: Invalid block_type = 0x%x\n", block_type);
		return 0;
	}
}

static inline uint16_t *phy_io_addr(void *phy, uint32_t addr)
{
	return phy + (map_phy_addr_space(addr) << 2);
}

static inline void phy_io_write16(uint16_t *phy, uint32_t addr, uint16_t data)
{
	out16(phy_io_addr(phy, addr), data);
}

static inline uint16_t phy_io_read16(uint16_t *phy, uint32_t addr)
{
	uint16_t reg = in16(phy_io_addr(phy, addr));
	return reg;
}

/* Read the DDR Phy Training values 1D and 2D.
 * Copy the filled structures to the destination memory
 * Calculate simple CRC for load checking
 */

int dump_phy_training_values(uint16_t **phy_ptr, uint32_t num_of_phy, void *dest)
{
    uint16_t *phy = NULL, value = 0x0;
	uint32_t size = 1, num_of_regs = 1, total = 0, crc;
	int i = 0, j = 0;

	for (j = 0; j < num_of_phy; j++) {
		/* Save training values of all PHYs */
		phy = phy_ptr[j];

		/* Enable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, 0x0);
		/* Enable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub |
				csr_ucclk_hclk_enables_addr, 0x3);

        size = sizeof(training_1D_values);
		num_of_regs = ARRAY_SIZE(training_1D_values);
		printf("1D Training reg val. size %d num_of_regs %d\n",size,num_of_regs);
		crc = 0;
		for (i = 1; i < num_of_regs; i++) {
			value = phy_io_read16(phy, training_1D_values[i].addr);
			training_1D_values[i].data = value;
//			printf("%3d. Reg: %x, value: %04x\n", i,
//					training_1D_values[i].addr, value);
			crc ^= training_1D_values[i].addr;
			crc ^= training_1D_values[i].data;
		}
		training_1D_values[0].data = crc & 0xffff;
		memcpy(dest, training_1D_values, size);
		total += size;
		dest += size;

		size = sizeof(training_2D_values);
		num_of_regs = ARRAY_SIZE(training_2D_values);
		printf("2D Training reg val. size %d num_of_regs %d\n",size,num_of_regs);
		crc = 0;
		for (i = 1; i < num_of_regs; i++) {
			value = phy_io_read16(phy, training_2D_values[i].addr);
			training_2D_values[i].data = value;
//			printf("%3d. Reg: %x, value: %04x\n", i,
//					training_2D_values[i].addr, value);
			crc ^= training_2D_values[i].addr;
			crc ^= training_2D_values[i].data;
		}
		training_2D_values[0].data = crc & 0xffff;
		memcpy(dest, training_2D_values, size);
		total += size;
		dest += size;

		/* Disable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub |
				csr_ucclk_hclk_enables_addr, 0x0);
		/* Disable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, 0x1);
	}
	printf("Total Bytes copied $%x\n",total);
	return 0;
}


int do_ddr(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	void *addr;
	uint16_t *phy[2];

	if (argc < 2)
		return cmd_usage(cmdtp);

	addr = (void *)simple_strtoul(argv[1], NULL, 16);

	/* 2 Phys to be read out */
	phy[0] = (void *)NXP_DDR_PHY1_ADDR;
	phy[1] = (void *)NXP_DDR_PHY2_ADDR;

	dump_phy_training_values(phy, 2, addr);

	return ret;
}

U_BOOT_CMD(
	ddr,	2,	1,	do_ddr,
	"LX2160 DDR-Phy read training data for 1D and 2D Training",
	"ddr destaddr\n"
)
