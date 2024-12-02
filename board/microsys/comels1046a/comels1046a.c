/*
 * SPDX-License-Identifier:    GPL-2.0+
 *
 * Copyright (C) 2020 MicroSys Electronics GmbH
 * Author: Kay Potthoff <kay.potthoff@microsys.de>
 *
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

DECLARE_GLOBAL_DATA_PTR;

#ifndef AQR105_IRQ_MASK
#define AQR105_IRQ_MASK 0x80000000
#endif

int checkboard(void)
{
    enum boot_src src = get_boot_src();

    puts("Board: COMe-LS1046A, boot from ");

    if (src == BOOT_SOURCE_SD_MMC)
        puts("SD\n");
    else if (src == BOOT_SOURCE_QSPI_NOR)
        puts("QSPI\n");

    return 0;
}

int board_early_init_f(void)
{

#ifdef CONFIG_SYS_I2C_EARLY_INIT
    i2c_early_init_f();
#endif
    fsl_lsch2_early_init_f();

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

int board_init(void)
{
    int rv, i;
    struct udevice *dev;

#ifdef CONFIG_ENV_IS_NOWHERE
    gd->env_addr = (ulong) & default_environment[0];
#endif

    struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
    enable_layerscape_ns_access();
#endif

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

#ifdef CONFIG_FSL_LS_PPA
    ppa_init();
#endif

    /* invert AQR105 IRQ pins polarity */
    out_be32(&scfg->intpcr, AQR105_IRQ_MASK);

    u32 reg = in_be32(&scfg->ecgtxcmcr);
    reg |= BIT(31-4); // use EC2_GTX_CLK125 as clock source
    out_be32(&scfg->ecgtxcmcr, reg);

    /*
     * Configure USB power fault for USB and USB2
     */
    reg = in_be32(&scfg->rcwpmuxcr0);

    reg &= ~(7<<(31-31)); // configure IIC4_SDA
    reg &= ~(7<<(31-27)); // configure IIC4_SCL

    reg &= ~(7<<(31-23));
    reg &= ~(7<<(31-19));
    reg |= (3<<(31-23)); // configure USB2_PWRFAULT
    reg |= (1<<(31-19)); // configure GPIO4[10]

    out_be32(&scfg->rcwpmuxcr0, reg);

    reg = in_be32(&scfg->usbpwrfault_selcr);
    reg &= ~(3<<SCFG_USBPWRFAULT_USB1_SHIFT);
    reg &= ~(3<<SCFG_USBPWRFAULT_USB2_SHIFT);
    reg &= ~(3<<SCFG_USBPWRFAULT_USB3_SHIFT);
    reg |= (SCFG_USBPWRFAULT_SHARED    << SCFG_USBPWRFAULT_USB1_SHIFT); // USB1 receives shared PWRFAULT
    reg |= (SCFG_USBPWRFAULT_DEDICATED << SCFG_USBPWRFAULT_USB2_SHIFT); // USB2 receives PWRFAULT from dedicated pin
    reg |= (SCFG_USBPWRFAULT_SHARED    << SCFG_USBPWRFAULT_USB3_SHIFT); // USB3 receives shared PWRFAULT
    out_be32(&scfg->usbpwrfault_selcr, reg);

    /*
     * Loop through all generic I2C devices and
     * initialize them:
     */
    i = 0;
    do {
        rv = uclass_get_device(UCLASS_I2C_GENERIC, i, &dev);
        if (rv==0) i++;
    } while (rv==0);

    return 0;
}

int last_stage_init(void)
{
    return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
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

    return 0;
}
