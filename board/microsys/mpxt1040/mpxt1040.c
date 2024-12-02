/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2015-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <hwconfig.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_fdt.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>
#include "sleep.h"
#include "mpxt1040.h"

DECLARE_GLOBAL_DATA_PTR;

int select_usb_clock(void)
{
    int ret;
    uchar value = 0x57;

    ret = i2c_write(I2C_IDT6V49205, 0x84, 1, &value, 1);
    if (ret) {
        puts("PCA: failed to select proper channel\n");
        return ret;
    }

    return 0;
}

int checkboard(void)
{
    printf("Board: MPXT1040\n");

    return 0;
}

int board_early_init_f(void)
{
#if defined(CONFIG_DEEP_SLEEP)
    if (is_warm_boot())
        fsl_dp_disable_console();
#endif

    return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_SYS_FLASH_BASE
    const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
    int flash_esel = find_tlb_idx((void *)flashbase, 1);

    /*
     * Remap Boot flash region to caching-inhibited
     * so that flash can be erased properly.
     */

    /* Flush d-cache and invalidate i-cache of any FLASH data */
    flush_dcache();
    invalidate_icache();

    if (flash_esel == -1) {
        /* very unlikely unless something is messed up */
        puts("Error: Could not find TLB for FLASH BASE\n");
        flash_esel = 2;         /* give our best effort to continue */
    } else {
        /* invalidate existing TLB entry for flash */
        disable_tlb(flash_esel);
    }

    set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
            MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G, 0, flash_esel,
            BOOKE_PAGESZ_256M, 1);
#endif
    select_usb_clock();

    return 0;
}

int misc_init_r(void)
{
    return 0;
}

int ft_board_setup(void *blob, bd_t * bd)
{
    phys_addr_t base;
    phys_size_t size;

    ft_cpu_setup(blob, bd);

    base = getenv_bootm_low();
    size = getenv_bootm_size();

    fdt_fixup_memory(blob, (u64) base, (u64) size);

#ifdef CONFIG_PCI
    pci_of_setup(blob, bd);
#endif

    fdt_fixup_liodn(blob);

#ifdef CONFIG_HAS_FSL_DR_USB
    fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
    fdt_fixup_fman_ethernet(blob);
#endif

    if (hwconfig("qe-tdm"))
        fdt_del_diu(blob);
    return 0;
}
