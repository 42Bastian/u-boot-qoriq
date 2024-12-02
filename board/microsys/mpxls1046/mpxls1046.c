/*
 * Copyright (C) 2018 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ppa.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>

#include <fsl_mdio.h>

#ifdef CONFIG_CARRIER_CRX06
#include "../crx06/crx06.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#define GPIO1_DIR    0x20000
#define GPIO1_14_DAT 0x20000

#define GPIO14_NR BIT(31-14)
#define GPIO15_NR BIT(31-15)

#ifndef AQR105_IRQ_MASK
#define AQR105_IRQ_MASK			0x80000000
#endif

int checkboard(void)
{
#ifdef CONFIG_TFABOOT
	enum boot_src src = get_boot_src();
#endif
	puts("Board: MPXLS1046, boot from ");

#ifdef CONFIG_SD_BOOT
	puts("SD\n");
#elif defined (CONFIG_NAND_BOOT)
	puts("NAND\n");
#elif CONFIG_QSPI_BOOT
	puts("QSPI\n");
#elif CONFIG_TFABOOT
	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD\n");
	if (src == BOOT_SOURCE_QSPI_NOR)
		puts("QSPI\n");
#endif

	return 0;
}

static int board_toggle_pcireset(void)
{
    setbits_be32((u32 *) CFG_SYS_LS_GPIO_DIR_ADDR, GPIO1_DIR);
    setbits_be32((u32 *) CFG_SYS_LS_GPIO_DAT_ADDR, GPIO1_14_DAT);
	return 0;
}

#ifdef CONFIG_CARRIER_CRX06
static int board_setup_qsgmii_mux(void)
{
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
    u32 srds_s1 =
        in_be32(&gur->rcwsr[4]) & FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	/*
	 * CRX06 Rev2:
	 * The GPIO pins 2_14 and 2_15 have now been connected to the two multiplexer
	 * that route the SerDes lanes from the CPU to the QSGMII PHYs. Depending on
	 * what the CPU/RCW configuration supports they have either to be set to
	 * 2xSGMII or to QSGMII.
	 * Note: set RCW[407] == 1 in order to make GPIO2[13:15] available.
	 */
    setbits_be32((u32 *) GPIO_DIR(2), GPIO14_NR | GPIO15_NR);   /* set to output */
    setbits_be32((u32 *) GPIO_IER(2), GPIO14_NR | GPIO15_NR);   /* mask interrupts */
    setbits_be32((u32 *) GPIO_DAT(2), GPIO14_NR | GPIO15_NR);   /* set both to 1 == 2xSGMII */

	if (srds_s1 == 0x1040) {
		/* set GPIO2_15 to 0 == 1xQSGMII */
        clrbits_be32((u32 *) GPIO_DAT(2), GPIO15_NR);
	}

	return 0;
}
#endif

int board_early_init_f(void)
{

#ifdef CONFIG_CARRIER_CRX06
	board_setup_qsgmii_mux();
#endif

#ifdef CONFIG_SYS_I2C_EARLY_INIT
    i2c_early_init_f();
#endif
	fsl_lsch2_early_init_f();

	board_toggle_pcireset();

#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;

	/* USB3 is not used, configure mux to IIC4_SCL/IIC4_SDA */
	out_be32(&scfg->rcwpmuxcr0, 0x3300);
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);
    usb_pwrfault =
        (SCFG_USBPWRFAULT_DEDICATED << SCFG_USBPWRFAULT_USB3_SHIFT) |
        (SCFG_USBPWRFAULT_DEDICATED << SCFG_USBPWRFAULT_USB2_SHIFT) |
        (SCFG_USBPWRFAULT_SHARED << SCFG_USBPWRFAULT_USB1_SHIFT);
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif
	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	/*
	 * Disable SD high speed capability. This would lead to a
	 * speed of 50MHz on SDHC bus, which is not supported on
	 * CRX05/CRX06.
	 */
	mmc->host_caps &= ~MMC_CAP(SD_HS);

	return 1;
}

int board_init(void)
{
#ifdef CONFIG_ENV_IS_NOWHERE
    gd->env_addr = (ulong) & default_environment[0];
#endif
    //struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

    //const u32 svr = gur_in32(&gur->svr);

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_NXP_ESBC
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

#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif

	/* invert AQR105 IRQ pins polarity */
	out_be32(&scfg->intpcr, AQR105_IRQ_MASK);

	return 0;
}

int last_stage_init(void)
{
	return 0;
}

int config_board_mux(void)
{
	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	config_board_mux();
	//printf("fdt_blob = %p sz=%ld\n", gd->new_fdt, gd->fdt_size);
	return 0;
}
#endif

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int banks = 1;

	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	/* fixup DT for the two DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;

	if (gd->bd->bi_dram[1].size > 0) {
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;
		banks++;
	}

	fdt_fixup_memory_banks(blob, base, size, banks);

	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

#ifdef CONFIG_CARRIER_CRX06
	board_crx06_fdt_fixup_phy(blob);
#endif

	return 0;
}
