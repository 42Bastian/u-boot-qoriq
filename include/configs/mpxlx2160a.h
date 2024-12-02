/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018,2021 NXP
 * (C) Copyright 2018 MicroSys Electronics GmbH
 */

#ifndef __MPXLX2160A_H
#define __MPXLX2160A_H

#include "lx2160a_common.h"

#define NON_EXTENDED_DUTCFG
#define CONFIG_LAST_STAGE_INIT
#define GPIO_BASE(N) ((CONFIG_SYS_IMMR + 0x01300000)+0x10000*((N)-1))
#define GPIO_PIN(N) BIT(31-(N))


#define GPIO_DIR(N) (GPIO_BASE(N)+0x00)
#define GPIO_DAT(N) (GPIO_BASE(N)+0x08)
#define GPIO_IER(N) (GPIO_BASE(N)+0x10)

/* The lowest and highest voltage allowed*/
#define VDD_MV_MIN			775
#define VDD_MV_MAX			855

/* PM Bus commands code for LTC3882*/
#define PMBUS_CMD_PAGE                  0x0
#define PMBUS_CMD_READ_VOUT             0x8B
#define PMBUS_CMD_PAGE_PLUS_WRITE       0x05
#define PMBUS_CMD_VOUT_COMMAND          0x21
#define PWM_CHANNEL0                    0x0

#define CONFIG_VOL_MONITOR_LTC3882_SET
#define CONFIG_VOL_MONITOR_LTC3882_READ

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


/* RTC */
#define CONFIG_SYS_RTC_BUS_NUM		4

/* MAC/PHY configuration */
#if defined(CONFIG_FSL_MC_ENET)
#define CONFIG_MII

#if defined(CONFIG_CARRIER_CRX08)
#define CONFIG_ETHPRIME		"DPMAC17@rgmii"

#define RGMII_PHY_ADDR1		0x00
#define RGMII_PHY_ADDR2		0x01

#define USXGMII_PHY_ADDR1		0x10
#define USXGMII_PHY_ADDR2		0x11
#endif

#endif

/* EEPROM */
#undef CONFIG_SYS_EEPROM_PAGE_WRITE_BITS
#undef CONFIG_SYS_I2C_EEPROM_ADDR_LEN
#undef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
#undef CONFIG_SYS_I2C_EEPROM_ADDR
#undef CONFIG_SYS_I2C_EEPROM_ADDR_LEN

#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_I2C_EEPROM_NXID_MAC 18
#define CONFIG_SYS_EEPROM_BUS_NUM		0
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
#define I2C_RETIMER_ADDR			0x18

#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE

#define CONFIG_ENV_ACCESS_IGNORE_FORCE
#define CONFIG_ENV_OFFSET          0x2000
#define CONFIG_ENV_SIZE            0x2000        /* 8KB */
#define CONFIG_I2C_ENV_EEPROM_BUS  CONFIG_SYS_EEPROM_BUS_NUM


#undef NXP_FSPI_FLASH_SIZE
/* FlexSPI */
#ifdef CONFIG_NXP_FSPI
#define NXP_FSPI_FLASH_SIZE SZ_64M
#define NXP_FSPI_FLASH_NUM  1
#endif

#define SD_MC_INIT_CMD				\
	"mmc read 0x80a00000 0x5000 0x1200;"	\
	"mmc read 0x80e00000 0x7000 0x800;"	\
	"env exists secureboot && "		\
	"mmc read 0x80640000 0x3200 0x20 && "	\
	"mmc read 0x80680000 0x3400 0x20 && "	\
	"esbc_validate 0x80640000 && "		\
	"esbc_validate 0x80680000 ;"		\
	"fsl_mc start mc 0x80a00000 0x80e00000\0"

#undef SD_BOOTCOMMAND
#define SD_BOOTCOMMAND						\
		"env exists mcinitcmd && mmcinfo; "		\
		"mmc read 0x80d00000 0x6800 0x800; "		\
		"env exists mcinitcmd && env exists secureboot "	\
		" && mmc read 0x806C0000 0x3600 0x20 "		\
		"&& esbc_validate 0x806C0000;env exists mcinitcmd "	\
		"&& fsl_mc apply dpl 0x80d00000;"		\
		"run sd_bootcmd;"		\
		"env exists secureboot && esbc_halt;"

#undef XSPI_NOR_BOOTCOMMAND
#define XSPI_NOR_BOOTCOMMAND						\
			"sf probe 0:0; "				\
			"sf read 0x806c0000 0x6c0000 0x40000; "		\
			"env exists mcinitcmd && env exists secureboot"	\
			" && esbc_validate 0x806c0000; "		\
			"sf read 0x80d00000 0xd00000 0x100000; "	\
			"env exists mcinitcmd && "			\
			"fsl_mc apply dpl 0x80d00000; "		\
			"run ram_bootcmd; "		\
			"env exists secureboot && esbc_halt;"

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND  SD_BOOTCOMMAND


#define BOOTARGS_SD0 \
    "bootargs_sd0=root=/dev/mmcblk0p1 rw rootwait iommu.passthrough=1 arm-smmu.disable_bypass=0\0"

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	EXTRA_ENV_SETTINGS			\
	"boot_scripts=lx2160ardb_boot.scr\0"	\
	"boot_script_hdr=hdr_lx2160ardb_bs.out\0"	\
	"BOARD=mpxlx2160a\0"			\
	BOOTARGS_SD0                                     \
	"mcinitcmd=mmc read 0x80a00000 0x5000 0x1200;"	\
	"mmc read 0x80e00000 0x7000 0x800;"	\
	"env exists secureboot && "		\
	"mmc read 0x80640000 0x3200 0x20 && "	\
	"mmc read 0x80680000 0x3400 0x20 && "	\
	"esbc_validate 0x80640000 && "		\
	"esbc_validate 0x80680000 ;"		\
	"fsl_mc start mc 0x80a00000 0x80e00000\0" \
	"sd_bootcmd=mmcinfo;mmc read 0x80d00000 0x6800 0x800; fsl_mc apply dpl 0x80d00000; setenv bootargs ${console} ${bootargs_sd0}; "		\
		"ext4load mmc 0:1 $load_addr /boot/fitImage.itb; "			\
		"bootm $load_addr\0"			\
	"ram_bootcmd=sf probe;sf read 0x80d00000 0xd00000 0x100000; fsl_mc apply dpl 0x80d00000; setenv bootargs ${console} ${bootargs}; "		\
		"sf read $load_addr 0xf00000 0x2000000; "			\
		"bootm $load_addr\0"		    \



#include <asm/fsl_secure_boot.h>

#endif /* __LX2_RDB_H */
