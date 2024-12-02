/* -*-C-*- */
/*
 * Copyright (C) 2018-2021 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#ifndef MICROSYS_COMMON_H
#define MICROSYS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
#ifndef CONFIG_PYLIBFDT
#define CONFIG_PYLIBFDT
#endif
#ifndef CONFIG_CMD_UPDATE_FIRMWARE_IMAGE
#define CONFIG_CMD_UPDATE_FIRMWARE_IMAGE
#endif

#ifdef CONFIG_FSL_QSPI

#ifndef CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_SPANSION
#endif
#ifndef CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_STMICRO
#endif
#undef FSL_QSPI_FLASH_SIZE
#define FSL_QSPI_FLASH_SIZE     SZ_16M
#undef  FSL_QSPI_FLASH_NUM
#define FSL_QSPI_FLASH_NUM      1

/*
 * Enable subsector access for QSPI flash.
 * The flash chip can handle subsector write and erase
 * operations of size 4KiB. This is enabled via setting
 * the define CONFIG_SPI_FLASH_USE_4K_SECTORS.
 */

#endif /* CONFIG_FSL_QSPI */

/*
 * Setup flash support:
 */
#ifndef CONFIG_FS_JFFS2
#define CONFIG_FS_JFFS2
#endif
#ifndef CONFIG_CMD_JFFS2
#define CONFIG_CMD_JFFS2
#endif
#ifndef CONFIG_CMD_NAND
#define CONFIG_CMD_NAND
#endif
#ifndef CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_MTDPARTS
#endif
#ifndef CONFIG_MTD
#define CONFIG_MTD
#endif
#ifndef CONFIG_DM_MTD
#define CONFIG_DM_MTD
#endif
#ifndef CONFIG_SPI_FLASH_MTD
#define CONFIG_SPI_FLASH_MTD
#endif
#define CONFIG_SYS_MAX_FLASH_BANKS 1
#define CONFIG_JFFS2_DEV "nand0"
#define CONFIG_MTD_PARTITIONS
#undef CONFIG_MTD_NOR_FLASH
#undef CONFIG_CMD_FLASH
#ifndef CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_NAND
#endif

#ifdef CONFIG_DM_I2C
#undef CONFIG_SYS_I2C
#else
#define CONFIG_SYS_I2C
#endif

#ifndef CONFIG_DM_I2C
#define CONFIG_SYS_I2C_EARLY_INIT
#endif

#undef CONFIG_ENV_IS_IN_MMC
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE

#undef  CONFIG_SYS_MMC_ENV_DEV
#undef CONFIG_CMD_EEPROM
#ifndef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_IS_IN_EEPROM
#endif

#define CONFIG_ENV_ACCESS_IGNORE_FORCE

#define CONFIG_SYS_MMC_ENV_DEV     0
#define CONFIG_ENV_OFFSET          0x2000
#define CONFIG_ENV_SIZE            0x2000        /* 8KB */
#define CONFIG_I2C_ENV_EEPROM_BUS  CONFIG_SYS_EEPROM_BUS_NUM

#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY 3

#define MTDIDS_DEFAULT   "nand0=nflash"
#define MTDPARTS_DEFAULT "mtdparts=nflash:-@0(rootfs)"

#ifdef CONFIG_TARGET_MPXLS1046
#define CONFIG_ROOTPATH        "/opt/fsl-image-networking-mpxls1046"
#endif

#ifdef CONFIG_TARGET_COMELS1046A
#define CONFIG_ROOTPATH     "/opt/microsys-image-networking-comels1046a"
#endif

#ifdef CONFIG_TARGET_MPXLS1043
#define CONFIG_ROOTPATH        "/opt/fsl-image-networking-mpxls1043"
#endif

#ifdef CONFIG_TARGET_MPXLS1043_2NAND
#define CONFIG_ROOTPATH        "/opt/fsl-image-networking-mpxls1043"
#endif

#if defined(CONFIG_PCI) && !defined(CONFIG_CMD_PCI)
#define CONFIG_CMD_PCI
#endif

#define CONFIG_BOOTFILE        "fitImage.itb"

#if defined(CONFIG_TARGET_MPXLS1043) \
	|| defined(CONFIG_TARGET_MPXLS1043_2NAND) \
    || defined(CONFIG_TARGET_MPXLS1046) \
    || defined(CONFIG_TARGET_COMELS1046A)

#define CFG_BOOTARGS_SD \
    "bootargs_sd=earlycon=uart8250,mmio,0x21c0500 root=/dev/mmcblk0p1 rw noinitrd console=ttyS0,115200 rootdelay=2"

#if defined(CONFIG_TARGET_COMELS1046A)
#define CFG_SDBOOTCOMMAND \
    "sdboot=setenv bootargs $bootargs_sd; ext4load mmc 0:1 $loadaddr boot/$bootfile; bootm $loadaddr"
#else
#define CFG_SDBOOTCOMMAND \
    "sdboot=setenv bootargs $bootargs_sd; ext4load mmc 0:1 $loadaddr boot/$bootfile; bootm $loadaddr$krncfg"
#endif


#if !defined(CONFIG_ARCH_LS1046A)

#define CFG_BOOTARGS_NAND \
    "bootargs_nand=earlycon=uart8250,mmio,0x21c0500 root=/dev/mtdblock0 rw noinitrd console=ttyS0,115200 rootfstype=jffs2"

#define CFG_NANDBOOTCOMMAND \
    "nandboot=setenv bootargs $bootargs_nand; sf probe; sf read $loadaddr 0x500000 0xb00000; bootm $loadaddr$krncfg"

#endif

#define CFG_BOOTARGS_QSPI \
    "bootargs_qspi=earlycon=uart8250,mmio,0x21c0500 root=/dev/mtdblock1 rw noinitrd console=ttyS0,115200 rootfstype=jffs2"

#define CFG_QSPIBOOTCOMMAND \
    "qspiboot=setenv bootargs $bootargs_qspi; nand read $loadaddr 0 0x1900000; bootm $loadaddr"

#undef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS    "earlycon=uart8250,mmio,0x21c0500"

#define CONFIG_NFSBOOTCOMMAND            \
    "setenv bootargs $bootargs root=/dev/nfs rw "    \
    "nfsroot=$serverip:$rootpath,v3,tcp "        \
    "ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
    "console=$consoledev,$baudrate $othbootargs;"    \
    "tftp $loadaddr ${tftppath}/$bootfile;"        \
    "bootm $loadaddr$krncfg"


#ifdef CONFIG_TFABOOT
#undef CONFIG_BOOTCOMMAND
#undef QSPI_NOR_BOOTCOMMAND
#undef SD_BOOTCOMMAND
#undef IFC_NAND_BOOTCOMMAND
#if defined(CONFIG_TARGET_COMELS1046A)
#define QSPI_NOR_BOOTCOMMAND "run qspiboot; "
#else
#define QSPI_NOR_BOOTCOMMAND "mmc rescan; run sdboot; "
#endif
#define SD_BOOTCOMMAND "mmc rescan;run sdboot; "
#define IFC_NAND_BOOTCOMMAND "mmc rescan;run sdboot; "
#endif

#endif


#ifdef CONFIG_TARGET_MPXLS1046

#define TFTP_PATH "tftppath=MPXLS1046/2021.08"

#ifdef CONFIG_CARRIER_CRX06
#define CONFIG_NETDEV "netdev=eth2"
#define KRNCFG "krncfg=#mpxls1046_crx06_xfi"
#ifdef CONFIG_MPX1046_RCW_SDCARD_QSGMII
#define KRNCFG "krncfg=#mpxls1046_crx06_qsgmii"
#endif
#else
#define KRNCFG "krncfg=#mpxls1046"
#define CONFIG_NETDEV "netdev=eth0"
#endif

#endif

#if defined(CONFIG_TARGET_COMELS1046A)
#define TFTP_PATH "tftppath=COMELS1046A/2021.08"
#define CONFIG_NETDEV "netdev=eth0"
#define KRNCFG "krncfg=#conf@microsys_comels1046a"
#endif

#ifdef CONFIG_TARGET_MPXLS1043

#define CONFIG_NETDEV "netdev=eth0"
#define KRNCFG "krncfg=#mpxls1043"
#define TFTP_PATH                \
    "tftppath=MPXLS1043/A2/2021.08"
#endif

#ifdef CONFIG_TARGET_MPXLS1043_2NAND

#define CONFIG_NETDEV "netdev=eth0"
#define KRNCFG "krncfg=#mpxls1043-2nand"
#define TFTP_PATH                \
    "tftppath=MPXLS1043/A2/2021.08"
#endif

#ifndef CFG_RAMDISK_IMAGE
#define CFG_RAMDISK_IMAGE \
    "ramdisk_bootfile=ramdisk_image.bin"
#endif
#ifndef CFG_BOOTARGS_RAM
#define CFG_BOOTARGS_RAM \
    "bootargs_ram=earlycon=uart8250,mmio,0x21c0500 root=/dev/ram0 rw console=ttyS0,115200"
#endif
#ifndef CONFIG_RAMBOOTCOMMAND
#define CONFIG_RAMBOOTCOMMAND \
    "setenv bootargs $bootargs_ram; tftp $loadaddr $tftppath/$ramdisk_bootfile; bootm $loadaddr"
#endif
#if defined(CONFIG_TARGET_MPXLS1043) \
	|| defined(CONFIG_TARGET_MPXLS1043_2NAND) \
    || defined(CONFIG_TARGET_MPXLS1046) \
    || defined(CONFIG_TARGET_COMELS1046A)
#undef CONFIG_EXTRA_ENV_SETTINGS
#if !defined(CONFIG_ARCH_LS1046A)
#define CONFIG_EXTRA_ENV_SETTINGS        \
    "hwconfig=fsl_ddr:bank_intlv=auto\0"    \
    "loadaddr=0xa0000000\0"            \
    "kernel_nand=0x00200000\0"        \
    "kernel_size=0x01700000\0"        \
    "consoledev=ttyS0\0"            \
    "fdt_high=0xffffffffffffffff\0"        \
    "initrd_high=0xffffffffffffffff\0" \
    "serverip=192.168.0.11\0" \
    "ipaddr=192.168.0.88\0" \
    CONFIG_NETDEV "\0" \
    KRNCFG "\0" \
    CFG_RAMDISK_IMAGE    "\0" \
    CFG_BOOTARGS_RAM    "\0" \
    CONFIG_RAMBOOTCOMMAND    "\0" \
    TFTP_PATH    "\0" \
    CFG_BOOTARGS_NAND "\0" \
    CFG_NANDBOOTCOMMAND "\0" \
    CFG_BOOTARGS_SD "\0" \
    CFG_SDBOOTCOMMAND "\0"
#else
#define CONFIG_EXTRA_ENV_SETTINGS       \
    "hwconfig=fsl_ddr:bank_intlv=auto\0"    \
    "loadaddr=0xa0000000\0"         \
    "kernel_nand=0x00200000\0"      \
    "kernel_size=0x01700000\0"      \
    "consoledev=ttyS0\0"            \
    "fdt_high=0xffffffffffffffff\0"     \
    "initrd_high=0xffffffffffffffff\0" \
    "serverip=192.168.0.11\0" \
    "ipaddr=192.168.0.88\0" \
    CONFIG_NETDEV "\0" \
    TFTP_PATH   "\0" \
    KRNCFG "\0" \
    CFG_RAMDISK_IMAGE   "\0" \
    CFG_BOOTARGS_RAM    "\0" \
    CONFIG_RAMBOOTCOMMAND   "\0" \
    CFG_BOOTARGS_QSPI "\0" \
    CFG_QSPIBOOTCOMMAND "\0" \
    CFG_BOOTARGS_SD "\0" \
    CFG_SDBOOTCOMMAND "\0"
#endif
#endif

#ifdef CONFIG_FSL_LSCH2
#define GPIO_BASE(N) ((0x2300000)+0x10000*((N)-1))
#define CFG_SYS_LS_GPIO_DIR_ADDR (0x2300000)
#define CFG_SYS_LS_GPIO_DAT_ADDR (0x2300008)
#define CFG_SYS_LS_GPIO2_DIR_ADDR (0x2310000)
#define CFG_SYS_LS_GPIO2_DAT_ADDR (0x2310008)
#endif

#ifdef CONFIG_FSL_LSCH3
#define GPIO_BASE(N) ((CONFIG_SYS_IMMR + 0x01300000)+0x10000*((N)-1))
#define GPIO_PIN(N) BIT(31-(N))
#endif

#define GPIO_DIR(N) (GPIO_BASE(N)+0x00)
#define GPIO_DAT(N) (GPIO_BASE(N)+0x08)
#define GPIO_IER(N) (GPIO_BASE(N)+0x10)

#undef CONFIG_SYS_BOOTM_LEN
#define CONFIG_SYS_BOOTM_LEN   (264 << 20)      /* Increase max gunzip size */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MICROSYS_COMMON_H */
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
