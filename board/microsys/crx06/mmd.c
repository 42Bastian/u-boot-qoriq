/* -*-C-*- */
/*
 * Copyright (C) 2017-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include "mmd.h"

#include <common.h>

#ifdef CONFIG_FMAN_ENET
#include <fm_eth.h>
#endif

#ifdef CONFIG_FSL_MC_ENET
#include <fsl-mc/ldpaa_wriop.h>
#endif

#define C45_CMD_REG (13)        // clause 22 register 13 as clause 45 command register
#define C45_AD_REG  (14)        // clause 22 register 14 as clause 45 address/data register

#define PWR_UP  (1)
#define PWR_DWN (0)

#define X33x0FW_MTD_HEADER_SIZE  (32)
#define X33x0FW_MTD_CKSUM_OFFSET (8)

typedef struct xmmd_reg_s {
    uint16_t devad;
    uint16_t reg;
} xmmd_reg_t;

static const xmmd_reg_t MV3310_TUNIT_CTRL1 = {.devad = MDIO_MMD_PMAPMD,.reg =
        0x0000
};

//static xmmd_reg_t MV3310_UNIT_CTRL          = {.devad=MDIO_MMD_PMAPMD,        .reg=0x0000};
//static xmmd_reg_t MV3310_UNIT_STAT          = {.devad=MDIO_MMD_PMAPMD,        .reg=0x0001};
//static const xmmd_reg_t MV3310_SERDES_CTRL_REG1   = {.devad=MDIO_MMD_PHYXS,         .reg=0xf003};
//static const xmmd_reg_t MV3310_LANE_CFG           = {.devad=MDIO_MMD_PHYXS,         .reg=0x8000};
//static const xmmd_reg_t MV3310_X_UNIT_CTRL1       = {.devad=MDIO_MMD_PCS,           .reg=0x1000};
//static const xmmd_reg_t MV3310_H_UNIT_CTRL1       = {.devad=MDIO_MMD_PHYXS,         .reg=0x1000};
static const xmmd_reg_t MV3310_SERDES_LANE0_CFG = {.devad = MDIO_MMD_PCS,.reg =
        0x0029
};

static const xmmd_reg_t MV3310_SERDES_LANE1_CFG = {.devad = MDIO_MMD_PCS,.reg =
        0x002a
};

//static const xmmd_reg_t MV3310_SERDES_CTRL_STATUS = {.devad=MDIO_MMD_AN,            .reg=0x800F};
static const xmmd_reg_t MV3310_MODE_CFG = {.devad =
        MV88X3310_DEVAD_C_UNIT,.reg = 0xf000
};

static const xmmd_reg_t MV3310_PORT_CTRL = {.devad =
        MV88X3310_DEVAD_C_UNIT,.reg = 0xf001
};

static const xmmd_reg_t MV3310_BOOT_STATUS = {.devad = MDIO_MMD_PMAPMD,.reg =
        0xc050
};

static const xmmd_reg_t LED0_CTRL = {.devad = MV88X3310_DEVAD_C_UNIT,.reg =
        0xf020
};

//static const xmmd_reg_t LED1_CTRL                 = {.devad=MV88X3310_DEVAD_C_UNIT, .reg=0xf021};
//static const xmmd_reg_t LED2_CTRL                 = {.devad=MV88X3310_DEVAD_C_UNIT, .reg=0xf022};
//static const xmmd_reg_t LED3_CTRL                 = {.devad=MV88X3310_DEVAD_C_UNIT, .reg=0xf023};

#if defined(CONFIG_CARRIER_CRX06) || defined(CONFIG_CARRIER_CRX08)
#if defined(CONFIG_MPX1046_RCW_SDCARD_2XXFI) \
    || defined(CONFIG_MPX1046_RCW_SDCARD_QSGMII) \
    || defined(CONFIG_MPX1088_RCW_SDCARD_CRX06_2QSGMII) \
    || defined(CONFIG_MPX1088_RCW_SDCARD_CRX06_2TSN) \
    || defined(CONFIG_MPX1046_RCW_QSPI_2XXFI) \
    || defined(CONFIG_TFABOOT)
#include "x3310fw.c"
#endif
#endif

static int mmd_set_register(mmd_cntx_t * cntx, uint16_t devad, uint16_t reg)
{
    if (cntx->__cached_devad != devad || cntx->__cached_reg != reg) {
        cntx->bus->write(cntx->bus, cntx->addr, MDIO_DEVAD_NONE, C45_CMD_REG,
                         devad & 0x1f);
        cntx->bus->write(cntx->bus, cntx->addr, MDIO_DEVAD_NONE, C45_AD_REG,
                         reg);
        cntx->__cached_devad = devad;
        cntx->__cached_reg = reg;
        return cntx->bus->write(cntx->bus, cntx->addr, MDIO_DEVAD_NONE,
                                C45_CMD_REG, (devad & 0x1f) | (1 << 14));
    }

    return 0;
}

int mmd_read(mmd_cntx_t * cntx, uint16_t devad, uint16_t reg)
{
    switch (cntx->mdio_proto) {
    case MDIO_CLAUSE_22:
        mmd_set_register(cntx, devad, reg);
        return cntx->bus->read(cntx->bus, cntx->addr, MDIO_DEVAD_NONE,
                               C45_AD_REG);
    case MDIO_CLAUSE_45:
        return cntx->bus->read(cntx->bus, cntx->addr, devad, reg);
    default:
        return -1;
    }
}

static int xmmd_read(mmd_cntx_t * cntx, const xmmd_reg_t * xreg)
{
    const int val = mmd_read(cntx, xreg->devad, xreg->reg);
    //    printf("%02d.%04x == %04x\n", xreg->devad, xreg->reg, val);
    return val;
}

int mmd_write(mmd_cntx_t * cntx, uint16_t devad, uint16_t reg, uint16_t val)
{
    switch (cntx->mdio_proto) {
    case MDIO_CLAUSE_22:
        mmd_set_register(cntx, devad, reg);
        return cntx->bus->write(cntx->bus, cntx->addr, MDIO_DEVAD_NONE,
                                C45_AD_REG, val);
    case MDIO_CLAUSE_45:
        return cntx->bus->write(cntx->bus, cntx->addr, devad, reg, val);
    default:
        return -1;
    }
}

static int xmmd_write(mmd_cntx_t * cntx, const xmmd_reg_t * xreg, uint16_t val)
{
    //    printf("%02d.%04x <= %04x\n", xreg->devad, xreg->reg, val);
    return mmd_write(cntx, xreg->devad, xreg->reg, val);
}

static int mv88x3310_port_sw_reset(mmd_cntx_t * cntx)
{
    int reg;
    reg = xmmd_read(cntx, &MV3310_PORT_CTRL);
    reg |= BIT(15);
    reg |= BIT(13);             // T unit software reset
    xmmd_write(cntx, &MV3310_PORT_CTRL, reg);
    do {
        udelay(100);
        reg = xmmd_read(cntx, &MV3310_PORT_CTRL);
    } while ((reg & (BIT(15) | BIT(13))) != 0);
    return 0;
}

static int mv88x3310_t_unit_sw_reset(mmd_cntx_t * cntx)
{
    int reg;
    reg = xmmd_read(cntx, &MV3310_TUNIT_CTRL1);
    reg |= BIT(15);
    xmmd_write(cntx, &MV3310_TUNIT_CTRL1, reg);
    udelay(120 * 1000);
    do {
        udelay(1000);
        reg = xmmd_read(cntx, &MV3310_TUNIT_CTRL1);
    } while ((reg & BIT(15)) != 0);
    return 0;
}

//static int mv88x3310_unit_sw_reset(mmd_cntx_t *cntx, const int devad)
//{
//    int reg;
//    MV3310_UNIT_CTRL.devad = devad;
//    reg = xmmd_read(cntx, &MV3310_UNIT_CTRL);
//    reg |= BIT(15);
//    xmmd_write(cntx, &MV3310_UNIT_CTRL, reg);
//    udelay(120*1000);
//    do {
//        udelay(1000);
//        reg = xmmd_read(cntx, &MV3310_UNIT_CTRL);
//    } while ((reg&BIT(15))!=0);
//    return 0;
//}

//static int mv88x3310_unit_wait_link(mmd_cntx_t *cntx, const int devad)
//{
//    int reg;
//    int count;
//    MV3310_UNIT_STAT.devad = devad;
//    xmmd_read(cntx, &MV3310_UNIT_STAT);
//    count = 100;
//    do {
//        udelay(1000);
//        reg = xmmd_read(cntx, &MV3310_UNIT_STAT);
//    } while ((reg&BIT(2))==0 && (count--) > 0);
//    return 0;
//}

//static int mv88x3310_x_unit_pwr(mmd_cntx_t *cntx, const int pwr)
//{
//    int reg;
//    reg = xmmd_read(cntx, &MV3310_X_UNIT_CTRL1);
//    if (pwr == PWR_UP) reg &= ~BIT(11);
//    else reg |= BIT(11);
//    return xmmd_write(cntx, &MV3310_X_UNIT_CTRL1, reg);
//}

//static int mv88x3310_h_unit_sw_reset(mmd_cntx_t *cntx)
//{
//    int reg;
//    reg = xmmd_read(cntx, &MV3310_H_UNIT_CTRL1);
//    reg |= BIT(15);
//    xmmd_write(cntx, &MV3310_H_UNIT_CTRL1, reg);
//    do {
//        udelay(100);
//        reg = xmmd_read(cntx, &MV3310_H_UNIT_CTRL1);
//    } while ((reg&BIT(15))!=0);
//    return 0;
//}

static int mv88x3310_t_unit_hw_reset(mmd_cntx_t * cntx)
{
    int reg;
    reg = xmmd_read(cntx, &MV3310_PORT_CTRL);
    reg |= BIT(12);
    xmmd_write(cntx, &MV3310_PORT_CTRL, reg);
    do {
        udelay(500);
        reg = xmmd_read(cntx, &MV3310_PORT_CTRL);
    } while ((reg & BIT(12)) != 0);
    return 0;
}

static int mv88x3310_set_mdio_download_mode(mmd_cntx_t * cntx)
{
    int reg;

    reg = mmd_read(cntx, MV88X3310_DEVAD_C_UNIT, 0xf008);
    if (reg < 0)
        return reg;
    reg |= BIT(5);
    return mmd_write(cntx, MV88X3310_DEVAD_C_UNIT, 0xf008, reg);
}

static int mv88x3310_set_start_address(mmd_cntx_t * cntx, uint32_t ram_address)
{
    mmd_write(cntx, MDIO_MMD_PCS, 0xd0f0, (uint16_t) (ram_address & 0xffff));
    return mmd_write(cntx, MDIO_MMD_PCS, 0xd0f1,
                     (uint16_t) ((ram_address >> 16) & 0xffff));
}

static int mv88x3310_get_firmware_version(mmd_cntx_t * cntx, uint8_t * maj,
                                          uint8_t * min, uint8_t * inc,
                                          uint8_t * test)
{
    int reg;

    reg = mmd_read(cntx, MDIO_MMD_PMAPMD, 0xc011);
    if (reg < 0)
        return reg;

    if (maj)
        *maj = (reg >> 8) & 0xff;
    if (min)
        *min = (reg) & 0xff;

    reg = mmd_read(cntx, MDIO_MMD_PMAPMD, 0xc012);

    if (inc)
        *inc = (reg >> 8) & 0xff;
    if (test)
        *test = (reg) & 0xff;

    return 0;
}

static inline int mv88x3310_read_checksum(mmd_cntx_t * cntx)
{
    return mmd_read(cntx, MDIO_MMD_PCS, 0xd0f3);
}

static inline int mv88x3310_get_media_select(mmd_cntx_t * cntx)
{
    return xmmd_read(cntx, &MV3310_MODE_CFG) & 7;
}

static int mv88x3310_mdio_download(mmd_cntx_t * cntx)
{
#if defined(CONFIG_MPX1046_RCW_SDCARD_2XXFI) \
    || defined(CONFIG_MPX1046_RCW_SDCARD_QSGMII) \
    || defined(CONFIG_MPX1088_RCW_SDCARD_CRX06_2QSGMII) \
    || defined(CONFIG_MPX1088_RCW_SDCARD_CRX06_2TSN) \
    || defined(CONFIG_MPX1046_RCW_QSPI_2XXFI) \
    || defined(CONFIG_TFABOOT)

    int reg, i, rv;
    uint16_t val, checksum, expected_checksum;
    uint16_t *fwimg;

    rv = mv88x3310_set_mdio_download_mode(cntx);
    if (rv < 0) {
        printf("Error: PHY@%02x on %s: cannot set MDIO download mode!\n",
               cntx->addr, cntx->bus->name);
        return -1;
    }

    mv88x3310_t_unit_hw_reset(cntx);

    reg = xmmd_read(cntx, &MV3310_BOOT_STATUS);

    if (reg != 0x000a) {
        printf("Error: PHY@%02x on %s is not in MDIO download mode!\n",
               cntx->addr, cntx->bus->name);
        return -1;
    }

    mv88x3310_read_checksum(cntx);  // clears checksum register

    mv88x3310_set_start_address(cntx, 0x00100000);

    reg = mmd_read(cntx, MDIO_MMD_PCS, 0xd00d);
    printf("\n%s PHY@%02x: Revision 0x%x %d %d/%d:\n", cntx->bus->name,
           cntx->addr, (((reg & 0xfc00) >> 6) >> 4) & 0x3f, reg & 0xf,
           ((reg & 0x70) >> 4) + 1, ((reg & 0x0380) >> 7) + 1);

    expected_checksum = ~((uint16_t) x3310fw[X33x0FW_MTD_CKSUM_OFFSET]
                          | (((uint16_t) x3310fw[X33x0FW_MTD_CKSUM_OFFSET + 1])
                             << 8));
    checksum = 0;

    fwimg = (uint16_t *) & x3310fw[X33x0FW_MTD_HEADER_SIZE];

    for (i = 0; i < ((X3310FW_LENGTH - X33x0FW_MTD_HEADER_SIZE) / 2); i++) {
        val = cpu_to_le16(fwimg[i]);
        mmd_write(cntx, MDIO_MMD_PCS, 0xd0f2, val);
        checksum += (val & 0xff) + ((val >> 8) & 0xff);
        if ((i % 512) == 0) {
            if (i > 0 && i % (512 * 64) == 0)
                putc('\n');
            putc('#');
        }
    }
    putc('\n');

    val = mv88x3310_read_checksum(cntx);

    //    printf("CHKSUM: expected=0x%04x calculated=0x%04x ram=0x%04x\n",
    //            expected_checksum, checksum, val);

    if (val != checksum) {
        printf
            ("Error: PHY@%02x on %s: checksum error when downloading firmware!\n",
             cntx->addr, cntx->bus->name);
        return -1;
    }

    if (val != expected_checksum) {
        printf("Error: PHY@%02x on %s: checksum error in firmware image!\n",
               cntx->addr, cntx->bus->name);
        return -1;
    }

    reg = xmmd_read(cntx, &MV3310_BOOT_STATUS);
    reg |= (1 << 6);
    xmmd_write(cntx, &MV3310_BOOT_STATUS, reg);

    udelay(100 * 1000);

    reg = xmmd_read(cntx, &MV3310_BOOT_STATUS);

    if ((reg & BIT(4)) == 0) {
        printf("Error: PHY@%02x on %s: application code did not start!\n",
               cntx->addr, cntx->bus->name);
        return -1;
    }

    if (reg & BIT(0)) {
        printf("Error: PHY@%02x on %s: boot code halted!\n", cntx->addr,
               cntx->bus->name);
        return -1;
    }

    return 0;
#else
    puts("WARN: MV88X3310: no firmware download configured!\n");
    return -1;
#endif
}

int mv88x3310_init(struct mii_dev *bus, const int addr,
                   const mdio_protocol_t proto)
{
    int reg, rv;
    uint8_t maj, min, inc, test;
    char *mac_type;
    INIT_MMD_CNTX(cntx, bus, addr);

    cntx.mdio_proto = proto;

    if (proto == MDIO_CLAUSE_22) {
        // set page 0:
        bus->write(bus, addr, MDIO_DEVAD_NONE, 22, 0);
    }

    rv = mv88x3310_get_firmware_version(&cntx, &maj, &min, &inc, &test);
    if (rv < 0) {
        printf
            ("Error: PHY@%02x on %s: cannot get firmware version information!\n",
             cntx.addr, cntx.bus->name);
        return -1;
    }

    if (maj == 0 && min == 0 && inc == 0 && test == 0) {
        mv88x3310_mdio_download(&cntx);
        mv88x3310_get_firmware_version(&cntx, &maj, &min, &inc, &test);
    }

    if (maj == 0xff && min == 0xff && inc == 0xff && test == 0xff) {
        /*
         * PHY is disconnected from bus...
         */
        return -1;
    } else {
        printf("%s PHY@%02x: Firmware version: %d.%d.%d.%d\n", cntx.bus->name,
               cntx.addr, maj, min, inc, test);
    }

    /*
     * On CRX06 rev1 the 10G-PHYs are connected to FSL_MDIO0 which is a 'classical'
     * standard MDIO bus. On such a bus the two PHYs need to be accessed using
     * clause 22 register layout. Additionally, on CRX06 rev1 the X-units of the
     * PHYs are connected to the XFI lanes of the processor. Therefore, the X-unit
     * needed to be re-configured to act as redundant host interface. To identify this
     * situation the code here checks whether a clause 22 access is desired or not.
     * This is supposed to work because on CRX06 rev2 the two PHYs are connected to FM_TGEC_MDIO,
     * which is the 10G MDIO bus. This bus uses clause 45 for accessing the PHYs. Configuring
     * redundant host mode is not necessary, because on CRX06 rev2 the H-units of the PHYs are
     * connected to the processor.
     */
    reg = xmmd_read(&cntx, &MV3310_MODE_CFG);
    reg &= ~7;                  // Copper only
    /*
     * Setup redundant host mode:
     */
    reg |= 4;                   // select redundant host mode
    if (proto == MDIO_CLAUSE_22)
        reg |= BIT(10);         // swaps redundant host lane select: active lane 0 with standby lane 1
    else
        reg &= ~BIT(10);        // no lane swapping
    reg |= (2 << 8);            // enable snooping (snoop data from network)
    xmmd_write(&cntx, &MV3310_MODE_CFG, reg);

    reg = xmmd_read(&cntx, &MV3310_PORT_CTRL);
    reg &= ~(3 << 3);
    reg |= (3 << 3);            // Fiber Type: 10GBase-R
    reg |= BIT(9);              // allow 10GBase-T and 10GBase-R to be on at the same time
    xmmd_write(&cntx, &MV3310_PORT_CTRL, reg);

    switch (reg & 0x07) {
        case 0x0:
            mac_type = "RXAUI/5.0GR/2.5GX/SGMII SGMII Auto-Neg On";
            break;
        case 0x1:
            mac_type = "XAUI with Rate Matching";
            break;
        case 0x2:
            mac_type = "RXAUI with Rate Matching";
            break;
        case 0x3:
            mac_type = "XAUI/5.0GR/2.5GX/SGMII Autoneg on";
            break;
        case 0x4:
            mac_type = "XFI/5.0GR/2.5GX/SGMII SGMII Auto-Neg On";
            break;
        case 0x5:
            mac_type = "XFI/5.0GR/2.5GX/SGMII SGMII Auto-Neg Off";
            break;
        case 0x6:
            mac_type = "XFI with Rate Matching";
            break;
        case 0x7:
            mac_type = "USXGMII";
            break;
    }

    printf("88X3310 MAC Type %s\n",mac_type);

    /*
     * Setup SERDES Tx amplitude
     *
     * Please check "88X3310(P), 88X3340(P) Rev A1 Release Notes"
     * section 6.8 "SERDES Tx Settings" for details:
     *
     * BIT[15:11] - pre tap
     * BIT[10:5]  - main tap
     * BIT[4:0]   - post tap
     *
     * 'main+pre+post' should not be greater than 40. 'main' cannot be
     * less than 20.
     */
    reg = mv88x3310_get_media_select(&cntx);
    xmmd_write(&cntx, &MV3310_SERDES_LANE0_CFG, 0x3a2);
    if (reg == 4) {             // == redundant host mode
        xmmd_write(&cntx, &MV3310_SERDES_LANE1_CFG, 0x3a2);
    } else {
        xmmd_write(&cntx, &MV3310_SERDES_LANE1_CFG, 0xb23);
    }

    mv88x3310_t_unit_sw_reset(&cntx);

    /*
     * Finally, two port resets are necessary:
     */
    mv88x3310_port_sw_reset(&cntx);
    mv88x3310_port_sw_reset(&cntx);

    reg = xmmd_read(&cntx, &LED0_CTRL);
    reg &= ~(0x1f << 3);
    reg |= 0x1 << 3;            // Set to 'Transmit or Receive Activity'
    reg &= ~3;                  // 00 = On drive LED low, Off - drive LED high
    reg &= ~BIT(2);             // Select blink rate
    xmmd_write(&cntx, &LED0_CTRL, reg);

    return 0;
}

#ifndef CONFIG_SPL_BUILD

#ifdef CONFIG_CARRIER_CRX06
int get_phy_id(struct mii_dev *bus, int addr, int devad, u32 * phy_id)
{
    int phy_reg;
    INIT_MMD_CNTX(cntx, bus, addr);

    //    printf("get_phy_id(%s, 0x%02x, %d)\n", bus->name, addr, devad);

#ifdef CONFIG_FSL_MC_ENET
#define C22_MDIO_NAME DEFAULT_WRIOP_MDIO1_NAME
#else
#define C22_MDIO_NAME DEFAULT_FM_MDIO_NAME
#endif

    /*
     * Check for 10Gb PHYs:
     */
    if (addr == 0x10 || addr == 0x11) {
        if (strncmp(bus->name, C22_MDIO_NAME, MDIO_NAME_LEN) == 0) {

            phy_reg = mmd_read(&cntx, MDIO_MMD_PMAPMD, MII_PHYSID1);

            if (phy_reg < 0)
                return -EIO;

            *phy_id = (phy_reg & 0xffff) << 16;

            phy_reg = mmd_read(&cntx, MDIO_MMD_PMAPMD, MII_PHYSID2);

            if (phy_reg < 0)
                return -EIO;

            *phy_id |= (phy_reg & 0xffff);

            return 0;
        } else {
            /* Grab the bits from PHYIR1, and put them
             * in the upper half */
            phy_reg = bus->read(bus, addr, MDIO_MMD_PMAPMD, MII_PHYSID1);

            if (phy_reg < 0)
                return -EIO;

            *phy_id = (phy_reg & 0xffff) << 16;

            /* Grab the bits from PHYIR2, and put them in the lower half */
            phy_reg = bus->read(bus, addr, MDIO_MMD_PMAPMD, MII_PHYSID2);

            if (phy_reg < 0)
                return -EIO;

            *phy_id |= (phy_reg & 0xffff);

            return 0;
        }
    }

    /* Grab the bits from PHYIR1, and put them
     * in the upper half */
    phy_reg = bus->read(bus, addr, devad, MII_PHYSID1);

    if (phy_reg < 0)
        return -EIO;

    *phy_id = (phy_reg & 0xffff) << 16;

    /* Grab the bits from PHYIR2, and put them in the lower half */
    phy_reg = bus->read(bus, addr, devad, MII_PHYSID2);

    if (phy_reg < 0)
        return -EIO;

    *phy_id |= (phy_reg & 0xffff);

    return 0;
}
#endif

#endif // CONFIG_SPL_BUILD

int board_phy_config(struct phy_device *phydev)
{
    if (phydev->drv->config) {
#ifdef DEBUG
        printf("%s PHY@%02x: 0x%08x %s\n", phydev->bus->name, phydev->addr,
               phydev->phy_id, phydev->drv->name);
#endif
        return phydev->drv->config(phydev);
    }
    return 0;
}

/*!@} */

/******************************************************************************
 * Local Variables:
 * mode: C
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * kate: space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
 * vim: set expandtab filetype=c:
 * vi: set et tabstop=4 shiftwidth=4: */
