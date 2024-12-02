// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018-2019 MicroSys Electronics GmbH
 *
 */
#include <common.h>
#include <i2c.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fsl_ddr.h>

#include <asm/io.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <fsl-mc/fsl_mc.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <asm/arch/ppa.h>
#include <hwconfig.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>

#include "../../freescale/common/vid.h"
#include <fsl_immap.h>
#include <fsl_esdhc.h>
#include <mmc.h>

//mcinitcmd=mmc read 0x80000000 0x5000 0x800;mmc read 0x80100000 0x7000 0x800;env exists secureboot && mmc read 0x80700000 0x3800 0x10 && mmc read 0x80740000 0x3A00 0x10 && esbc_validate 0x80700000 && esbc_validate 0x80740000 ;fsl_mc start mc 0x80000000 0x80100000
//mcmemsize=0x70000000
//mmc read 0x80200000 0x3800 0x800;fsl_mc lazyapply dpl 0x80200000
/*
 *            U-Boot 0x0800       -0x1000
 * 0x80740000 DPL 0x3800 14336
 * 0x80000000 MC     0x5000 20480 (sz=0xdaa58/1748blk, 20480-22228)
 * 0x80100000 DPC    0x7000 28672
 * 0x80740000 DPL 0x6800 14336
 */

DECLARE_GLOBAL_DATA_PTR;

static int board_toggle_pcireset(void)
{
    setbits_le32((u32 *) GPIO_DIR(3), GPIO_PIN(30));
    setbits_le32((u32 *) GPIO_DAT(3), GPIO_PIN(30));
    return 0;
}

#ifdef CONFIG_CARRIER_CRX06

int board_setup_qsgmii_mux(void)
{
    struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
    u32 srds_s1, cfg;

    cfg =
        in_le32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]) &
        FSL_CHASSIS3_SRDS1_PRTCL_MASK;
    cfg >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;

    srds_s1 = serdes_get_number(FSL_SRDS_1, cfg);

    /*
     * CRX06 Rev2:
     * The GPIO pins 2_7 and 2_16 have now been connected to the two multiplexer
     * that route the SerDes lanes from the CPU to the QSGMII PHYs. Depending on
     * what the CPU/RCW configuration supports they can either be set to
     * 2xSGMII or to 2xQSGMII.
     */
    setbits_le32((u32 *) GPIO_DIR(2), GPIO_PIN(7) | GPIO_PIN(16));  /* set to output */
    setbits_le32((u32 *) GPIO_IER(2), GPIO_PIN(7) | GPIO_PIN(16));  /* mask interrupts */

    if (srds_s1 == 0x1d) {      // == 1144
        clrbits_le32((u32 *) GPIO_DAT(2), GPIO_PIN(7)); /* set to 0 == QSGMII */
        puts("GPIO2_7:  QSGMII1\n");
    } else {
        setbits_le32((u32 *) GPIO_DAT(2), GPIO_PIN(7)); /* set to 1 == SGMII */
        puts("GPIO2_7:  SGMII3\n");
    }

    if (srds_s1 == 0x1d || srds_s1 == 0x19) {   // == 1144 or 1143
        clrbits_le32((u32 *) GPIO_DAT(2), GPIO_PIN(16));    /* set to 0 == QSGMII */
        puts("GPIO2_16: QSGMII2\n");
    } else {
        setbits_le32((u32 *) GPIO_DAT(2), GPIO_PIN(16));    /* set to 1 == SGMII */
        puts("GPIO2_16: SGMII7\n");
    }

    return 0;
}
#endif

int board_early_init_f(void)
{
    fsl_lsch3_early_init_f();

    board_toggle_pcireset();

    return 0;
}


int checkboard(void)
{

	enum boot_src src = get_boot_src();
	puts("Board: MPXLS1088, boot from ");
	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD\n");
	if (src == BOOT_SOURCE_QSPI_NOR)
		puts("QSPI\n");

    return 0;
}


#if !defined(CONFIG_SPL_BUILD)
#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_CARRIER_CRX06
    board_setup_qsgmii_mux();
#endif

    return 0;
}
#endif
#endif

int get_serdes_volt(void)
{
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

#if !defined(CONFIG_SPL_BUILD)
int board_init(void)
{
    init_final_memctl_regs();
#if defined(CONFIG_TARGET_MPXLS1088) && defined(CONFIG_FSL_MC_ENET)
    u32 __iomem *irq_ccsr = (u32 __iomem *) ISC_BASE;
#endif

#ifdef CONFIG_ENV_IS_NOWHERE
    gd->env_addr = (ulong) & default_environment[0];
#endif

#if defined(CONFIG_TARGET_MPXLS1088) && defined(CONFIG_FSL_MC_ENET)
    /* invert AQR105 IRQ pins polarity */
    out_le32(irq_ccsr + IRQCR_OFFSET / 4, AQR105_IRQ_MASK);
#endif

#ifdef CONFIG_FSL_LS_PPA
    ppa_init();
#endif

    return 0;
}

void detail_board_ddr_info(void)
{
    puts("\nDDR    ");
    print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
    print_ddr_info(0);
}

#ifdef CONFIG_FSL_MC_ENET
void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}

void fdt_fixup_board_enet(void *fdt)
{
    int offset;

    offset = fdt_path_offset(fdt, "/soc/fsl-mc");

    if (offset < 0)
        offset = fdt_path_offset(fdt, "/fsl-mc");

    if (offset < 0)
        offset = fdt_path_offset(fdt, "/soc/fsl-mc");

    if (offset < 0) {
        printf("%s: ERROR: fsl-mc node not found in device tree (error %d)\n",
               __func__, offset);
        return;
    }

    if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0))
        fdt_status_okay(fdt, offset);
    else
        fdt_status_fail(fdt, offset);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
void fsl_fdt_fixup_flash(void *fdt)
{
    int offset;
#ifdef CONFIG_TFABOOT
    u32 __iomem *dcfg_ccsr = (u32 __iomem *) DCFG_BASE;
    u32 val;
#endif

    /*
     * IFC-NOR and QSPI are muxed on SoC.
     * So disable IFC node in dts if QSPI is enabled or
     * disable QSPI node in dts in case QSPI is not enabled.
     */

#ifdef CONFIG_TFABOOT
    enum boot_src src = get_boot_src();
    bool disable_ifc = false;

    switch (src) {
    case BOOT_SOURCE_IFC_NOR:
        disable_ifc = false;
        break;
    case BOOT_SOURCE_QSPI_NOR:
        disable_ifc = true;
        break;
    default:
        val = in_le32(dcfg_ccsr + DCFG_RCWSR15 / 4);
        if (DCFG_RCWSR15_IFCGRPABASE_QSPI == (val & (u32) 0x3))
            disable_ifc = true;
        break;
    }

    if (disable_ifc == true) {
        offset = fdt_path_offset(fdt, "/soc/ifc/nor");

        if (offset < 0)
            offset = fdt_path_offset(fdt, "/ifc/nor");
    } else {
        offset = fdt_path_offset(fdt, "/soc/quadspi");

        if (offset < 0)
            offset = fdt_path_offset(fdt, "/quadspi");
    }

#else
#ifdef CONFIG_FSL_QSPI
    offset = fdt_path_offset(fdt, "/soc/ifc/nor");

    if (offset < 0)
        offset = fdt_path_offset(fdt, "/ifc/nor");
#else
    offset = fdt_path_offset(fdt, "/soc/quadspi");

    if (offset < 0)
        offset = fdt_path_offset(fdt, "/quadspi");
#endif
#endif
    if (offset < 0)
        return;

    fdt_status_disabled(fdt, offset);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int i;
	bool mc_memory_bank = false;

	u64 *base;
	u64 *size;
	u64 mc_memory_base = 0;
	u64 mc_memory_size = 0;
	u16 total_memory_banks;

	ft_cpu_setup(blob, bd);

	fdt_fixup_mc_ddr(&mc_memory_base, &mc_memory_size);

	if (mc_memory_base != 0)
		mc_memory_bank = true;

	total_memory_banks = CONFIG_NR_DRAM_BANKS + mc_memory_bank;

	base = calloc(total_memory_banks, sizeof(u64));
	size = calloc(total_memory_banks, sizeof(u64));

	/* fixup DT for the two GPP DDR banks */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
#endif

	if (mc_memory_base != 0) {
		for (i = 0; i <= total_memory_banks; i++) {
			if (base[i] == 0 && size[i] == 0) {
				base[i] = mc_memory_base;
				size[i] = mc_memory_size;
				break;
			}
		}
	}

	fdt_fixup_memory_banks(blob, base, size, total_memory_banks);

	fdt_fsl_mc_fixup_iommu_map_entry(blob);

	fsl_fdt_fixup_flash(blob);

#ifdef CONFIG_FSL_MC_ENET
	fdt_fixup_board_enet(blob);
#endif

    return 0;
}
#endif
#endif /* defined(CONFIG_SPL_BUILD) */

#ifdef CONFIG_TFABOOT
#ifdef CONFIG_MTD_NOR_FLASH
int is_flash_available(void)
{
    char *env_hwconfig = env_get("hwconfig");
    enum boot_src src = get_boot_src();
    int is_nor_flash_available = 1;

    switch (src) {
    case BOOT_SOURCE_IFC_NOR:
        is_nor_flash_available = 1;
        break;
    case BOOT_SOURCE_QSPI_NOR:
        is_nor_flash_available = 0;
        break;
        /*
         * In Case of SD boot,if qspi is defined in env_hwconfig
         * disable nor flash probe.
         */
    default:
        if (hwconfig_f("qspi", env_hwconfig))
            is_nor_flash_available = 0;
        break;
    }
    return is_nor_flash_available;
}
#endif

void *env_sf_get_env_addr(void)
{
    return (void *)(CONFIG_SYS_FSL_QSPI_BASE + CONFIG_ENV_OFFSET);
}
#endif
