/*
 * Copyright 2017-2018 NXP
 * Copyright (C) 2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __MPXLS1028_H
#define __MPXLS1028_H

#include "ls1028a_common.h"

//#ifdef CONFIG_NAND
//#undef CONFIG_NAND
//#endif
//
//#ifdef CONFIG_CMD_NAND
//#undef CONFIG_CMD_NAND
//#endif
//
//#ifdef CONFIG_MTD_RAW_NAND
//#undef CONFIG_MTD_RAW_NAND
//#endif
//
//#ifdef CONFIG_SYS_NAND_USE_FLASH_BBT
//#undef CONFIG_SYS_NAND_USE_FLASH_BBT
//#endif
//
//#ifdef CONFIG_SYS_NAND_SELF_INIT
//#undef CONFIG_SYS_NAND_SELF_INIT
//#endif

#ifndef CONFIG_SYS_MAX_NAND_DEVICE
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#endif

#define CONFIG_SYS_CLK_FREQ    100000000
#define CONFIG_DDR_CLK_FREQ    100000000
#define COUNTER_FREQUENCY_REAL (CONFIG_SYS_CLK_FREQ/4)

/* DDR */
#define CONFIG_SYS_DDR_RAW_TIMING
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE             0xdeadbeef
#define CONFIG_DIMM_SLOTS_PER_CTLR        1

#ifdef CONFIG_DDR_SPD
#undef CONFIG_DDR_SPD
#endif
#ifdef CONFIG_SPD_EEPROM
#undef CONFIG_SPD_EEPROM
#endif

#undef CONFIG_NR_DRAM_BANKS
#ifdef CONFIG_MPXLS1028_MEM_4G
#define CONFIG_NR_DRAM_BANKS 2
#else
#define CONFIG_NR_DRAM_BANKS 1
#endif

#ifndef CONFIG_PYLIBFDT
#define CONFIG_PYLIBFDT
#endif
#ifndef CONFIG_CMD_UPDATE_FIRMWARE_IMAGE
#define CONFIG_CMD_UPDATE_FIRMWARE_IMAGE
#endif

#define CONFIG_SYS_MAX_FLASH_BANKS 1

#ifndef CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH
#endif

#ifdef CONFIG_SPI_FLASH
#ifndef CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_MACRONIX
#endif
#ifndef CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_SPANSION
#endif
#ifndef CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_STMICRO
#endif
#ifndef CONFIG_SPI_FLASH_USE_4K_SECTORS
#define CONFIG_SPI_FLASH_USE_4K_SECTORS
#endif
#ifndef CONFIG_SPI_FLASH_MTD
#define CONFIG_SPI_FLASH_MTD
#endif
#endif



#ifdef CONFIG_DM_I2C
#undef CONFIG_SYS_I2C
#else
#define CONFIG_SYS_I2C
#endif

/* RTC */
#undef CONFIG_RTC_PCF2127
#undef CONFIG_SYS_I2C_RTC_ADDR
#undef CONFIG_SYS_RTC_BUS_NUM
#ifndef CONFIG_RTC_PCF85063
#define CONFIG_RTC_PCF85063
#endif
#define CONFIG_SYS_RTC_BUS_NUM  0
#define CONFIG_SYS_I2C_RTC_ADDR 0x51
#ifndef CONFIG_CMD_DATE
#define CONFIG_CMD_DATE
#endif

/* EEPROM */
#undef  CONFIG_SYS_I2C_EEPROM_ADDR
#undef  CONFIG_SYS_I2C_EEPROM_ADDR_LEN
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_I2C_EEPROM_NXID_MAC        10
#define CONFIG_SYS_EEPROM_BUS_NUM             0
#undef CONFIG_SYS_I2C_EEPROM_BUS
#define CONFIG_SYS_I2C_EEPROM_BUS             CONFIG_SYS_EEPROM_BUS_NUM
#define CONFIG_SYS_I2C_EEPROM_ADDR            0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN        2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS     3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5

#define I2C_RETIMER_ADDR 0x18

/* FlexSPI */
#ifdef CONFIG_NXP_FSPI
#define NXP_FSPI_FLASH_SIZE SZ_16M
#define NXP_FSPI_FLASH_NUM  1
#endif

#undef CONFIG_ENV_SIZE

/* Store environment at top of flash */
#ifdef CONFIG_EMU_PXP
#define CONFIG_ENV_SIZE 0x1000
#else
#define CONFIG_ENV_SIZE 0x2000
#endif

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MONITOR_BASE CONFIG_SPL_TEXT_BASE
#else
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE
#endif

#ifndef CONFIG_DM_I2C
#define CONFIG_SYS_I2C_EARLY_INIT
#endif


/* SATA */
#ifndef SPL_NO_SATA
#ifndef CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT2
#endif
#ifndef CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4
#endif
#define CONFIG_SYS_SCSI_MAX_SCSI_ID 1
#define CONFIG_SYS_SCSI_MAX_LUN     1
#define CONFIG_SYS_SCSI_MAX_DEVICE  (CONFIG_SYS_SCSI_MAX_SCSI_ID *\
                        CONFIG_SYS_SCSI_MAX_LUN)
#define SCSI_VEND_ID                0x1b4b
#define SCSI_DEV_ID                 0x9170
#define CONFIG_SCSI_DEV_LIST        {SCSI_VEND_ID, SCSI_DEV_ID}
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SATA1            AHCI_BASE_ADDR1
#endif

/* FIXME: in later ppa.itb this is called "config-1" */
#define SEC_FIRMEWARE_FIT_CNF_NAME "config@1"

#define CONFIG_LAST_STAGE_INIT

#undef CONFIG_ENV_IS_IN_MMC
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_ACCESS_IGNORE_FORCE
#undef CONFIG_CMD_EEPROM
#ifndef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_IS_IN_EEPROM
#endif
#define CONFIG_ENV_OFFSET          0x2000
#define CONFIG_ENV_SIZE            0x2000        /* 8KB */
#define CONFIG_I2C_ENV_EEPROM_BUS CONFIG_SYS_EEPROM_BUS_NUM

#define CONFIG_LOADADDR        0xa0000000
#undef  SD_BOOTCOMMAND
#undef  SD2_BOOTCOMMAND
#undef  XSPI_NOR_BOOTCOMMAND
#undef  QSPI_NOR_BOOTCOMMAND
#undef  CONFIG_BOOTCOMMAND
#undef  CONFIG_SYS_MMC_ENV_DEV

#define SD_BOOTCOMMAND       "run bootcmd_sd"
#define SD2_BOOTCOMMAND      "run bootcmd_emmc"
#define XSPI_NOR_BOOTCOMMAND "run bootcmd_xspi_nor"

#define SYS_SPI_BOOT_DEV     "0:0"

#ifdef CONFIG_TFABOOT_SOURCE_SD_MMC
#define CONFIG_BOOTCOMMAND     SD_BOOTCOMMAND
#define CONFIG_SYS_MMC_ENV_DEV 0
#define SYS_MMC_BOOT_DEV       0
#elif CONFIG_TFABOOT_SOURCE_SD_MMC2
#define CONFIG_BOOTCOMMAND     SD2_BOOTCOMMAND
#define CONFIG_SYS_MMC_ENV_DEV 1
#define SYS_MMC_BOOT_DEV       1
#else
#define CONFIG_BOOTCOMMAND     XSPI_NOR_BOOTCOMMAND
#define CONFIG_SYS_MMC_ENV_DEV 0
#define SYS_MMC_BOOT_DEV       0
#endif

#define CONFIG_ETHPRIME "enetc-0"

#undef  CONFIG_BOOTARGS
#define CONFIG_BOOTARGS \
    "console=ttyS0,115200 earlycon=uart8250,mmio,0x21c0500"

#define CONSOLE      \
    "console=console=ttyS0,115200 earlycon=uart8250,mmio,0x21c0500\0"
#define HDPLOAD_SD   \
    "hdpload_sd=ext4load mmc ${mmcdev}:1 ${load_addr} boot/ls1028a-dp-fw.bin; hdp load ${load_addr} ${filesize}\0"
#define HDPLOAD_NOR   \
    "hdpload_nor=sf probe ${spidev}; sf read ${load_addr} 0x40000 0x20000; hdp load ${load_addr} 0x20000\0"
#define BOOTARGS_SD0 \
    "bootargs_sd0=root=/dev/mmcblk0p1 rw rootwait iommu.passthrough=1 arm-smmu.disable_bypass=0\0"
#define BOOTARGS_SD1 \
    "bootargs_sd1=root=/dev/mmcblk1p1 rw rootwait iommu.passthrough=1 arm-smmu.disable_bypass=0\0"
#define BOOTFIT_SD0  \
    "bootfit_sd0=setenv bootargs ${console} ${bootargs_sd0} ${videomode}; ext4load mmc ${mmcdev}:1 ${load_addr} boot/fitImage.itb; bootm ${load_addr}#${kconfig}\0"
#define BOOTFIT_SD1  \
    "bootfit_sd1=setenv bootargs ${console} ${bootargs_sd1} ${videomode}; ext4load mmc ${mmcdev}:1 ${load_addr} boot/fitImage.itb; bootm ${load_addr}#${kconfig}\0"
#define VIDEOMODE    \
    "videomode=video=1920x1080-32@60 cma=256M\0"
#define VIDEOMODE4K  \
    "videomode4k=video=3840x2160-32@60 cma=256M\0"
#define BOOTCMD_SD   \
    "bootcmd_sd=setenv mmcdev 0; run hdpload_sd; run bootfit_sd${mmcdev}\0"
#define BOOTCMD_SD2   \
    "bootcmd_emmc=setenv mmcdev 1; run hdpload_sd; run bootfit_sd${mmcdev}\0"
#define BOOTCMD_XSPI_NOR   \
    "bootcmd_xspi_nor=run hdpload_nor; run bootfit_sd${mmcdev}\0"

#undef  CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS                    \
    "load_addr=" __stringify(CONFIG_LOADADDR) "\0"   \
    "mmcdev=" __stringify(SYS_MMC_BOOT_DEV) "\0"     \
    "spidev=" SYS_SPI_BOOT_DEV "\0"                  \
    "kconfig=conf-microsys_mpxls1028-crx07.dtb\0" \
    "flash_bl2=ext4load mmc ${mmcdev}:1 ${loadaddr} boot/bl2_flexspi_nor.pbl; sf probe ${spidev}; sf update ${loadaddr} 0 ${filesize} \0" \
    "flash_bl3=ext4load mmc ${mmcdev}:1 ${loadaddr} boot/fip_uboot.bin; sf probe ${spidev}; sf update ${loadaddr} 100000 ${filesize} \0" \
    "flash_dp=ext4load mmc ${mmcdev}:1 ${loadaddr} boot/ls1028a-dp-fw.bin; sf probe ${spidev}; sf update ${loadaddr} 0x40000 ${filesize} \0" \
    BOOTCMD_SD                                       \
    BOOTCMD_SD2                                      \
    BOOTCMD_XSPI_NOR                                 \
    CONSOLE                                          \
    HDPLOAD_SD                                       \
    HDPLOAD_NOR                                      \
    BOOTARGS_SD0                                     \
    BOOTFIT_SD0                                      \
    BOOTARGS_SD1                                     \
    BOOTFIT_SD1                                      \
    VIDEOMODE4K                                      \
    VIDEOMODE

#ifdef CONFIG_SECURE_BOOT
#include <asm/fsl_secure_boot.h>
#endif

#endif /* __MPXLS1028_H */
