/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021-2023 NXP
 */

#ifndef __LA1238_RDB_H
#define __LA1238_RDB_H

#include "lx2160a_common.h"

#define PCAL_BUS_NO		2
#define PCAL_BUS_NO_REVB	3
#define BOOT_FROM_XSPI		1
#define BOOT_FROM_EMMC		2
#define BOOT_FROM_SD		3
#define BOOT_FROM_XSPI_MODEM	1
#define BOOT_FROM_PCIE_MODEM	2
#define BOOT_FROM_PEB_MODEM	3
#define PCAL_CPU_ADDR		0x20
#define PCAL_MODEM_ADDR		0x21
#define PCAL_INPUT_PORT		0x00
#define PCAL_OUTPUT_PORT	0x01
#define PCAL_POL_INV		0x02
#define PCAL_CONFIG		0x03
#define PCAL_ODS_0		0x40
#define PCAL_ODS_1		0x41
#define PCAL_INPUT_LATCH	0x42
#define PCAL_PU_PD_ENABLE	0x43
#define PCAL_PU_PD_SEL		0x44
#define PCAL_INT_MASK		0x45
#define PCAL_INT_STATUS		0x46
#define PCAL_OUT_PORT_CONFIG	0x47

#undef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1)	\
	func(MMC, mmc, 0)
/*
 * Need to override existing (lx2160a) with la1238-rdb so set_board_info will
 * use proper prefix when creating full board_name (SYS_BOARD + type)
 */
#define REVA			0x00
#define REVB			0x01

#undef CONFIG_SYS_NXP_SRDS_3

/* Make channel 0 default */
#undef I2C_MUX_CH_DEFAULT
#define I2C_MUX_CH_DEFAULT		1

/* VID */
#define I2C_MUX_CH_VOL_MONITOR		0x2
/* Voltage monitor on channel 2*/
#define I2C_VOL_MONITOR_ADDR		0x63
#define I2C_VOL_MONITOR_BUS_V_OFFSET	0x2
#define I2C_VOL_MONITOR_BUS_V_OVF	0x1
#define I2C_VOL_MONITOR_BUS_V_SHIFT	3

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

/* The lowest and highest voltage allowed*/
#define VDD_MV_MIN			775
#define VDD_MV_MAX			925

/* PM Bus commands code for LTC3882*/
#define PMBUS_CMD_PAGE                  0x0
#define PMBUS_CMD_READ_VOUT             0x8B
#define PMBUS_CMD_PAGE_PLUS_WRITE       0x05
#define PMBUS_CMD_VOUT_COMMAND          0x21
#define PWM_CHANNEL0                    0x0

/* RTC */
#define CONFIG_SYS_RTC_BUS_NUM		0

/* MAC/PHY configuration */
#if defined(CONFIG_FSL_MC_ENET)
#define CONFIG_ETHPRIME		"DPMAC3@usxgmii"
#define AQR113_PHY_ADDR		0x8
#define AQR113_IRQ_MASK		0x800 /* IRQ-11 */
#define TI_DS250_I2C_ADDR	0x1
#define INPHI_PHY_ADDR1		0x0
#define INPHI_PHY_ADDR2		0x1
#ifdef CONFIG_SD_BOOT
#define IN112525_FW_ADDR	0x980000
#else
#define IN112525_FW_ADDR	0x20980000
#endif
#define IN112525_FW_LENGTH	0x40000
#endif

/* EMC2305 */
#define I2C_MUX_CH_EMC2305			0x09
#define I2C_EMC2305_ADDR			0x4D
#define I2C_EMC2305_CMD				0x40
#define I2C_EMC2305_PWM				0x80

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	EXTRA_ENV_SETTINGS			\
	"boot_scripts=la1238rdb_boot.scr\0"	\
	"boot_script_hdr=hdr_la1238rdb_bs.out\0"	\
	"xspi_bootcmd=echo Trying load from flexspi..;"		\
		"sf probe 0:0 && sf read $load_addr "		\
		"$kernel_start $kernel_size ; env exists secureboot &&"	\
		"sf read $kernelheader_addr_r $kernelheader_start "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		" bootm $load_addr#$board\0"			\
	"sd_bootcmd=echo Trying load from sd card..;"		\
		"mmc dev 0;mmcinfo; mmc read $load_addr "	\
		"$kernel_addr_sd $kernel_size_sd ;"		\
		"env exists secureboot && mmc read $kernelheader_addr_r "\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "	\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$board\0"				\
	"emmc_bootcmd=echo Trying load from emmc card..;"	\
		"mmc dev 1; mmcinfo; mmc read $load_addr "	\
		"$kernel_addr_sd $kernel_size_sd ;"		\
		"env exists secureboot && mmc read $kernelheader_addr_r "\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "	\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$board\0"			\
	"othbootargs=default_hugepagesz=1024m hugepagesz=1024m"	\
		" hugepages=2 mem=13726M pulse_width_1588=100\0"

#include <asm/fsl_secure_boot.h>

#endif /* __LA1238_RDB_H */
