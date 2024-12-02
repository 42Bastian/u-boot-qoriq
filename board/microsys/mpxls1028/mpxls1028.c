// SPDX-License-Identifier:    GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright (C) 2019 MicroSys Electronics GmbH
 */

#include <common.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <hwconfig.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <env_internal.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#ifdef CONFIG_FSL_LS_PPA
#include <asm/arch/ppa.h>
#endif
#include <fsl_immap.h>
#include <netdev.h>
#include <video_fb.h>
#include <fsl_esdhc.h>
#include <rtc.h>
#include <fdtdec.h>
#include <miiphy.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO2_BASE_ADDR  (CONFIG_SYS_IMMR + 0x01310000)
#define GPIO2_GPDIR_ADDR (GPIO2_BASE_ADDR + 0x0)
#define GPIO2_GPDAT_ADDR (GPIO2_BASE_ADDR + 0x8)
#define GPIO2_GPIBE_ADDR (GPIO2_BASE_ADDR + 0x18)

static const unsigned int gpin_mask(const unsigned int pin)
{
    return (1 << (31 - pin));
}

#ifdef CONFIG_LPUART
u32 get_lpuart_clk(void)
{
    return gd->bus_clk / CONFIG_SYS_FSL_LPUART_CLK_DIV;
}
#endif

int board_init(void)
{
//    struct udevice *gpio2_dev;
//    struct gpio_desc desc;
    uint32_t dat;

#define GPIO2_BASE 0x02310000

#ifdef CONFIG_ENV_IS_NOWHERE
    gd->env_addr = (ulong) & default_environment[0];
#endif

#ifdef CONFIG_FSL_LS_PPA
    ppa_init();
#endif

#ifndef CONFIG_SYS_EARLY_PCI_INIT

//    uclass_get_device_by_name(UCLASS_GPIO, "gpio@2310000", &gpio2_dev);
//
//    if (gpio2_dev) {
//        desc.dev = gpio2_dev;
//
//        gpio_direction_output();
//    }
//    else {
    /*
     * Define GPIO2[29,6,7] as outputs:
     */
    setbits(le32, GPIO2_GPDIR_ADDR,
            gpin_mask(29)
            | gpin_mask(6) | gpin_mask(7));

    /*
     * Define GPIO2[28] as input:
     */
    clrbits(le32, GPIO2_GPDIR_ADDR, gpin_mask(28));

    dat = in_le32(GPIO2_GPDAT_ADDR);

    /*
     * Release PCI reset:
     * set GPIO2[29] to high
     */
    dat |= gpin_mask(29);

    /*
     * Switch off user LEDs LD1 and LD2:
     */
    dat &= ~(gpin_mask(6) | gpin_mask(7));

    out_le32(GPIO2_GPDAT_ADDR, dat);

    /*
     * Enable input buffer for GPIOs in use:
     */
    setbits(le32, GPIO2_GPIBE_ADDR,
            gpin_mask(31) /* WD_TRIG#  */
            | gpin_mask(29) /* PCIE_RST# */
            | gpin_mask(28) /* EXPD_IRQ# */
            | gpin_mask(7)  /* LD1       */
            | gpin_mask(6)  /* LD2       */);

//    }

    /* run PCI init to kick off ENETC */
    pci_init();

#endif

#ifndef CONFIG_DM_RTC
    rtc_init();
#endif

    return 0;
}

int board_eth_init(struct bd_info *bis)
{
    return pci_eth_init(bis);
}

int board_early_init_f(void)
{

#ifdef CONFIG_SYS_I2C_EARLY_INIT
    i2c_early_init_f();
#endif

    fsl_lsch3_early_init_f();
    return 0;
}

static uint8_t get_board_rev()
{
    struct udevice *dev;
    static uchar reg = 0xff;

    if ((reg==0xff) && (i2c_get_chip_for_busnum(0, 0x43, 1, &dev)==0)) {
        if (dm_i2c_read(dev, 0x0f, &reg, 1)==0) {
            reg = (((reg&BIT(7))>>7) | ((reg&BIT(6))>>5) | ((reg&BIT(5))>>3)) + 1;
        }
    }

    return reg;
}

int checkboard(void)
{
    printf("Board: Rev. %d\n", get_board_rev());

    return 0;
}

void detail_board_ddr_info(void)
{
    puts("\nDDR    ");
    print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
    print_ddr_info(0);
}

#ifdef CONFIG_OF_BOARD_SETUP

/*
 * Hardware default stream IDs are 0x4000 + PCI function #, but that's outside
 * the acceptable range for SMMU.  Use Linux DT values instead or at least
 * smaller defaults.
 */
#define ECAM_NUM_PFS            7
#define ECAM_IERB_BASE          0x1F0800000
#define ECAM_PFAMQ(pf, vf)      ((ECAM_IERB_BASE + 0x800 + (pf) * \
                      0x1000 + (vf) * 4))
/* cache related transaction attributes for PCIe functions */
#define ECAM_IERB_MSICAR        (ECAM_IERB_BASE + 0xa400)
#define ECAM_IERB_MSICAR_VALUE      0x30

/* number of VFs per PF, VFs have their own AMQ settings */
static const u8 enetc_vfs[ECAM_NUM_PFS] = { 2, 2 };

void setup_ecam_amq(void *blob)
{
    int streamid, sid_base, off;
    int pf, vf, vfnn = 1;
    u32 iommu_map[4];
    int err;

    /*
     * Look up the stream ID settings in the DT, if found apply the values
     * to HW, otherwise use HW values shifted down by 4.
     */
    off = fdt_node_offset_by_compatible(blob, 0, "pci-host-ecam-generic");
    if (off < 0) {
        debug("ECAM node not found\n");
        return;
    }

    err = fdtdec_get_int_array(blob, off, "iommu-map", iommu_map, 4);
    if (err) {
        sid_base = in_le32(ECAM_PFAMQ(0, 0)) >> 4;
        debug("\"iommu-map\" not found, using default SID base %04x\n",
              sid_base);
    } else {
        sid_base = iommu_map[2];
    }
    /* set up AMQs for all integrated PCI functions */
    for (pf = 0; pf < ECAM_NUM_PFS; pf++) {
        streamid = sid_base + pf;
        out_le32(ECAM_PFAMQ(pf, 0), streamid);

        /* set up AMQs for VFs, if any */
        for (vf = 0; vf < enetc_vfs[pf]; vf++, vfnn++) {
            streamid = sid_base + ECAM_NUM_PFS + vfnn;
            out_le32(ECAM_PFAMQ(pf, vf + 1), streamid);
        }
    }
}

void setup_ecam_cacheattr(void)
{
    /* set MSI cache attributes */
    out_le32(ECAM_IERB_MSICAR, ECAM_IERB_MSICAR_VALUE);
}

#define IERB_PFMAC(pf, vf, n)       (ECAM_IERB_BASE + 0x8000 + (pf) * \
                     0x100 + (vf) * 8 + (n) * 4)

static int ierb_fno_to_pf[] = {0, 1, 2, -1, -1, -1, 3};

/* ENETC Port MAC address registers, accepts big-endian format */
static void ierb_set_mac_addr(int fno, const u8 *addr)
{
    u16 lower = *(const u16 *)(addr + 4);
    u32 upper = *(const u32 *)addr;

    if (ierb_fno_to_pf[fno] < 0)
        return;

    out_le32(IERB_PFMAC(ierb_fno_to_pf[fno], 0, 0), upper);
    out_le32(IERB_PFMAC(ierb_fno_to_pf[fno], 0, 1), (u32)lower);
}

/* copies MAC addresses in use to IERB so Linux can also use them */
void setup_mac_addr(void *blob)
{
    struct eth_pdata *plat;
    struct udevice *it;
    struct uclass *uc;
    int fno, offset;
    u32 portno;
    char path[256];
    ofnode node;

	node = ofnode_path("/mscc_felix");
	if (!ofnode_valid(node)) {
		debug("No mscc_felix node found\n");
		return 0;
	}

    uclass_get(UCLASS_ETH, &uc);
    uclass_foreach_dev(it, uc) {
        if (!it->driver || !it->driver->name)
            continue;
        if (!strcmp(it->driver->name, "enetc_eth")) {
            /* PFs use the same addresses in Linux and U-Boot */
            plat = dev_get_plat(it);
            if (!plat)
                continue;

            fno = PCI_FUNC(pci_get_devfn(it));
            ierb_set_mac_addr(fno, plat->enetaddr);
        } else if (!strcmp(it->driver->name, "felix-port")) {
            /* Switch ports should also use the same addresses */
            plat = dev_get_plat(it);
            if (!plat)
                continue;
            if (!ofnode_valid(node))
                continue;
            if (ofnode_read_u32(node, "reg", &portno))
                continue;
            sprintf(path,
                "/soc/pcie@1f0000000/switch@0,5/ports/port@%d",
                (int)portno);
            offset = fdt_path_offset(blob, path);
            if (offset < 0)
                continue;
            fdt_setprop(blob, offset, "mac-address",
                    plat->enetaddr, 6);
        }
    }
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
    u64 base[CONFIG_NR_DRAM_BANKS];
    u64 size[CONFIG_NR_DRAM_BANKS];
    int i;

    ft_cpu_setup(blob, bd);

    /* fixup DT for the two GPP DDR banks */
    for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
        base[i] = gd->bd->bi_dram[i].start;
        size[i] = gd->bd->bi_dram[i].size;
    }

#ifdef CONFIG_RESV_RAM
    /* reduce size if reserved memory is within this bank */
    if (gd->arch.resv_ram >= base[0] && gd->arch.resv_ram < base[0] + size[0])
        size[0] = gd->arch.resv_ram - base[0];
#if CONFIG_NR_DRAM_BANKS > 1
    else if (gd->arch.resv_ram >= base[1]
             && gd->arch.resv_ram < base[1] + size[1])
        size[1] = gd->arch.resv_ram - base[1];
#endif
#endif

    fdt_fixup_memory_banks(blob, base, size, CONFIG_NR_DRAM_BANKS);

    fdt_fixup_icid(blob);

    setup_ecam_amq(blob);
    setup_ecam_cacheattr();
    setup_mac_addr(blob);

    return 0;
}

#endif

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
    uchar reg;
    struct udevice *dev;

    if (i2c_get_chip_for_busnum(0, 0x4c, 1, &dev)==0) {

        /*
         * Configure TMP451
         */

        dm_i2c_read(dev, 0x03, &reg, 1);
        reg |= (1<<7); // set MASK1 => disable ALERT#
        reg |= (1<<5); // set THERM2# mode
        dm_i2c_write(dev, 0x09, &reg, 1);

    }
    else printf("Can't find i2c-%d-%02x!\n", 0, 0x4c);

    return 0;
}
#endif

void *video_hw_init(void)
{
    return NULL;
}

#ifdef CONFIG_EMMC_BOOT
void *esdhc_get_base_addr(void)
{
    return (void *)CONFIG_SYS_FSL_ESDHC1_ADDR;
}

#endif
