// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ppa.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_sec.h>
#include <fsl_ddr_sdram.h>
#include <power/mc34vr500_pmic.h>
#include "../common/i2c_mux.h"

#define LS1046A_PORSR1_REG 0x1EE0000
#define BOOT_SRC_SD        0x20000000
#define BOOT_SRC_MASK	   0xFF800000
#define BOARD_REV_BIT0_GPIO2	13
#define BOARD_REV_BIT1_GPIO2	14
#define BOARD_REV_BIT2_GPIO3	26
#define BOARD_REV_BIT3_GPIO3	27
#define M2_PWR_EN_MASK	   0x00000100
#define PHY1_RST_MASK	   0x40000
#define PHY2_RST_MASK	   0x100000
#define RESET_TCLK_MASK	   0x00400000	/* GPIO3_9*/

#define BYTE_SWAP_32(word)  ((((word) & 0xff000000) >> 24) |  \
(((word) & 0x00ff0000) >>  8) | \
(((word) & 0x0000ff00) <<  8) | \
(((word) & 0x000000ff) << 24))

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

#ifdef CONFIG_TFABOOT
int fsl_initdram(void)
{
	gd->ram_size = tfa_get_dram_size();

	if (!gd->ram_size)
		gd->ram_size = fsl_ddr_sdram_size();

	return 0;
}
#else
#error "CONFIG_TFABOOT is not defined"
#endif

#ifndef CONFIG_SPL_BUILD
static inline uint8_t get_board_version(void)
{
	u8 bit0, bit1, bit2, bit3, rev;
	struct ccsr_gpio *pgpio;

	pgpio = (void *)(GPIO2_BASE_ADDR);
	bit0 = (in_be32(&pgpio->gpdat) >> (31 - BOARD_REV_BIT0_GPIO2)) & 0x1;
	bit1 = (in_be32(&pgpio->gpdat) >> (31 - BOARD_REV_BIT1_GPIO2)) & 0x1;

	pgpio = (void *)(GPIO3_BASE_ADDR);
	bit2 = (in_be32(&pgpio->gpdat) >> (31 - BOARD_REV_BIT2_GPIO3)) & 0x1;
	bit3 = (in_be32(&pgpio->gpdat) >> (31 - BOARD_REV_BIT3_GPIO3)) & 0x1;

	rev = (bit3 << 3) | (bit2 << 2) | (bit1 << 1) | bit0;
	return rev;
}

int checkboard(void)
{
	static const char *freq[2] = {"100.00MHZ", "125.00MHZ"};
	u32 boot_src;

		u8 rev;

	rev = get_board_version();
	switch (rev) {
	case 0x0:
		puts("Board: LS1046ATP1, Rev: A, boot from ");
		break;
	case 0x1:
		puts("Board: LS1046ATP1, Rev: B, boot from ");
		break;
	case 0x2:
		puts("Board: LS1046ATP1, Rev: C, boot from ");
		break;
	default:
		puts("Board: LS1046ATP1, Rev: Unknown, boot from ");
		break;
	}
	boot_src = BYTE_SWAP_32(readl(LS1046A_PORSR1_REG));

	if ((boot_src & BOOT_SRC_MASK) == BOOT_SRC_SD)
		puts("SD\n");
	else
		puts("QSPI\n");
	printf("SD1_CLK2 = %s, SD2_CLK1 = %s\n", freq[1], freq[0]);

	return 0;
}

int board_init(void)
{
#ifdef CONFIG_SECURE_BOOT
	/*
	 * In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	 */
	u32 val;

	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT,0);
	return 0;
}

void config_board_mux(void)
{
	u32 val;
	struct ccsr_gpio *pgpio = (void *)(GPIO3_BASE_ADDR);

#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;

	out_be32(&scfg->rcwpmuxcr0, 0x3300);
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB2);
	usb_pwrfault = (SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB3_SHIFT) |
			(SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB2_SHIFT) |
			(SCFG_USBPWRFAULT_SHARED <<
			SCFG_USBPWRFAULT_USB1_SHIFT);
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif

	/* M2_PWR_EN for GCT modem */
	val = in_be32(&pgpio->gpdir);
	val |=  M2_PWR_EN_MASK;
	out_be32(&pgpio->gpdir, val);

	val = in_be32(&pgpio->gpdat);
	val |=  M2_PWR_EN_MASK;
	out_be32(&pgpio->gpdat, val);

}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	config_board_mux();
	/* Writing SCFG_COREBCR register Enhance IO strength of SDHC_CLK */
	out_le32(0x1570d04, 0x80b6dabe);
	return 0;
}
#endif

int ft_board_setup(void *blob,struct  bd_info *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	/* fixup DT for the two DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

	fdt_fixup_memory_banks(blob, base, size, 2);
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

	fdt_fixup_icid(blob);

	return 0;
}
#endif
