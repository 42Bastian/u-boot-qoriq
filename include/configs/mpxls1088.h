/*
 * Copyright 2017 NXP
 * Copyright 2018 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MPXLS1088_H
#define __MPXLS1088_H

#include "ls1088a_common.h"

/*Activate to build Binman Tool*/
#ifndef CONFIG_PYLIBFDT
#define CONFIG_PYLIBFDT
#endif

#ifndef SPL_NO_BOARDINFO
#ifndef CONFIG_DISPLAY_BOARDINFO_LATE
#define CONFIG_DISPLAY_BOARDINFO_LATE
#endif
#endif

#ifndef CONFIG_MISC_INIT_R
#define CONFIG_MISC_INIT_R
#endif
#undef CONFIG_NR_DRAM_BANKS
#define CONFIG_NR_DRAM_BANKS		2
#define CONFIG_FSL_DDR_BIST	/* enable built-in memory test */
#define CONFIG_FSL_DDR_INTERACTIVE	/* Interactive debugging */


/*
 * PPA Configuration
 */
#undef CONFIG_CHAIN_OF_TRUST

#define CONFIG_LOADADDR 0xa0000000

#if defined(CONFIG_QSPI_BOOT) || defined(CONFIG_SD_BOOT_QSPI)
#define SYS_NO_FLASH
#undef CONFIG_CMD_IMLS
#endif

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000
#define COUNTER_FREQUENCY_REAL		25000000	/* 25MHz */
#define COUNTER_FREQUENCY		25000000	/* 25MHz */

#define CONFIG_SYS_DDR_RAW_TIMING
#ifdef CONFIG_EMU
#define CONFIG_SYS_FSL_DDR_EMU
#define CONFIG_SYS_MXC_I2C1_SPEED	40000000
#define CONFIG_SYS_MXC_I2C2_SPEED	40000000
#else
#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef
#endif
#undef SPD_EEPROM_ADDRESS
#define CONFIG_DIMM_SLOTS_PER_CTLR	1


#ifdef CONFIG_MPX1088_RCW_SDCARD_CRX06_2QSGMII
#undef CONFIG_SYS_FSL_HAS_RGMII
#endif


/* SATA */
#ifndef CONFIG_DM_SCSI
#define CONFIG_DM_SCSI
#endif
#ifndef CONFIG_LIBATA
#define CONFIG_LIBATA
#endif
#ifndef CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI
#endif
#define CONFIG_SCSI_AHCI_PLAT

#define CONFIG_SYS_SATA1			AHCI_BASE_ADDR1

#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
										CONFIG_SYS_SCSI_MAX_LUN)


#if !defined(CONFIG_QSPI_BOOT) && !defined(CONFIG_SD_BOOT_QSPI)
#define CONFIG_SYS_NOR0_CSPR_EXT	(0x0)
#define CONFIG_SYS_NOR_AMASK		IFC_AMASK(128 * 1024 * 1024)
#define CONFIG_SYS_NOR_AMASK_EARLY	IFC_AMASK(64 * 1024 * 1024)

#define CONFIG_SYS_NOR0_CSPR					\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS)		| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR0_CSPR_EARLY				\
	(CSPR_PHYS_ADDR(CONFIG_SYS_FLASH_BASE_PHYS_EARLY)	| \
	CSPR_PORT_SIZE_16					| \
	CSPR_MSEL_NOR						| \
	CSPR_V)
#define CONFIG_SYS_NOR_CSOR	CSOR_NOR_ADM_SHIFT(6)
#define CONFIG_SYS_NOR_FTIM0	(FTIM0_NOR_TACSE(0x1) | \
				FTIM0_NOR_TEADC(0x1) | \
				FTIM0_NOR_TEAHC(0x1))
#define CONFIG_SYS_NOR_FTIM1	(FTIM1_NOR_TACO(0x1) | \
				FTIM1_NOR_TRAD_NOR(0x1))
#define CONFIG_SYS_NOR_FTIM2	(FTIM2_NOR_TCS(0x0) | \
				FTIM2_NOR_TCH(0x0) | \
				FTIM2_NOR_TWP(0x1))
#define CONFIG_SYS_NOR_FTIM3	0x04000000
#define CONFIG_SYS_IFC_CCR	0x01000000

#ifndef SYS_NO_FLASH
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_FLASH_QUIET_TEST
#define CONFIG_FLASH_SHOW_PROGRESS	45 /* count down from 45/5: 9..1 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#endif
#endif

#ifndef SPL_NO_IFC
#define CONFIG_NAND_FSL_IFC
#endif

#define CONFIG_SYS_NAND_MAX_ECCPOS	256
#define CONFIG_SYS_NAND_MAX_OOBFREE	2

#define CONFIG_SYS_NAND_CSPR_EXT	(0x0)
#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8 /* Port Size = 8 bit */ \
				| CSPR_MSEL_NAND	/* MSEL = NAND */ \
				| CSPR_V)
#define CONFIG_SYS_NAND_AMASK	IFC_AMASK(64 * 1024)

#define CONFIG_SYS_NAND_CSOR    (CSOR_NAND_ECC_ENC_EN   /* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN  /* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4  /* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3Byes */ \
				| CSOR_NAND_PGS_2K	/* Page Size = 2K */ \
				| CSOR_NAND_SPRZ_64/* Spare size = 64 */ \
				| CSOR_NAND_PB(64))	/*Pages Per Block = 64*/

#define CONFIG_SYS_NAND_ONFI_DETECTION

/* ONFI NAND Flash mode0 Timing Params */
#define CONFIG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x07) | \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x07) | \
					FTIM0_NAND_TWH(0x0a))
#define CONFIG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x32) | \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0x0e)   | \
					FTIM1_NAND_TRP(0x18))
#define CONFIG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0x0f) | \
					FTIM2_NAND_TREH(0x0a) | \
					FTIM2_NAND_TWHRE(0x1e))
#define CONFIG_SYS_NAND_FTIM3		0x0

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE

#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)

#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NAND_FTIM3

#define CONFIG_SYS_LS_MC_BOOT_TIMEOUT_MS 5000

#define CONFIG_VID_FLS_ENV              "mpxls1088_vdd_mv"
#undef CONFIG_VID

#ifndef SPL_NO_RTC
/*
* RTC configuration
*
#define RTC
#define CONFIG_RTC_PCF8563 1
#define CONFIG_SYS_I2C_RTC_ADDR         0x32
#define CONFIG_SYS_RTC_BUS_NUM  0
#define CONFIG_CMD_DATE*/
#endif


/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_EEPROM_BUS_NUM		0
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_I2C_EEPROM_NXID_MAC 12

#ifndef SPL_NO_QSPI
/* QSPI device */
#ifndef CONFIG_FSL_QSPI
#define CONFIG_FSL_QSPI
#endif
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MAX_HZ		1000000
#define CONFIG_ENV_SPI_MODE		0x03
#endif

#define CONFIG_CMD_MEMINFO
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x9fffffff

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MONITOR_BASE CONFIG_SPL_TEXT_BASE
#else
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE
#endif
#define CONFIG_ENV_ACCESS_IGNORE_FORCE
#define CONFIG_FSL_MEMAC

#ifndef SPL_NO_ENV
/* Initial environment variables */
#ifdef CONFIG_TFABOOT
#define QSPI_MC_INIT_CMD				\
	"sf probe 0:0;sf read 0x80a00000 0xA00000 0x200000;"	\
	"sf read 0x80100000 0xE00000 0x100000;"				\
	"sf read 0x88000000 0xD00000 0x100000;" \
	"env exists secureboot && "			\
	"sf read 0x80700000 0x700000 0x40000 && "	\
	"sf read 0x80740000 0x740000 0x40000 && "	\
	"esbc_validate 0x80700000 && "			\
	"esbc_validate 0x80740000 ;"			\
	"fsl_mc start mc 0x80a00000 0x80100000;"	\
	"fsl_mc lazyapply dpl 0x88000000\0"
#define SD_MC_INIT_CMD				\
	"mmcinfo;mmc read 0x80a00000 0x5000 0x1000;"		\
	"mmc read 0x88000000 0x6800 0x2000;" \
	"mmc read 0x80100000 0x7000 0x2000;"				\
	"env exists secureboot && "			\
	"mmc read 0x80700000 0x3800 0x20 && "		\
	"mmc read 0x80740000 0x3A00 0x20 && "		\
	"esbc_validate 0x80700000 && "			\
	"esbc_validate 0x80740000 ;"			\
	"fsl_mc start mc 0x80a00000 0x80100000;"	\
	"fsl_mc lazyapply dpl 0x88000000\0"

#else
#if defined(CONFIG_QSPI_BOOT)
#define MC_INIT_CMD				\
	"mcinitcmd=sf probe 0:0;sf read 0x80000000 0xA00000 0x100000;"	\
	"sf read 0x80100000 0xE00000 0x100000;"				\
	"env exists secureboot && "			\
	"sf read 0x80700000 0x700000 0x40000 && "	\
	"sf read 0x80740000 0x740000 0x40000 && "	\
	"esbc_validate 0x80700000 && "			\
	"esbc_validate 0x80740000 ;"			\
	"fsl_mc start mc 0x80000000 0x80100000\0"	\
	"mcmemsize=0x70000000\0"
#elif defined(CONFIG_SD_BOOT)
#define MC_INIT_CMD				\
	"mcinitcmd=mmcinfo;mmc read 0x80000000 0x5000 0x800;"		\
	"mmc read 0x80100000 0x7000 0x800;"				\
	"env exists secureboot && "			\
	"mmc read 0x80700000 0x3800 0x20 && "		\
	"mmc read 0x80740000 0x3A00 0x20 && "		\
	"esbc_validate 0x80700000 && "			\
	"esbc_validate 0x80740000 ;"			\
	"fsl_mc start mc 0x80000000 0x80100000\0"	\
	"mcmemsize=0x70000000\0"
#endif
#endif /* CONFIG_TFABOOT */

#define CFG_BOOTARGS_SD \
	"bootargs_sd=earlycon=uart8250,mmio,0x21c0500 root=/dev/mmcblk0p1 rw noinitrd console=ttyS0,115200 rootdelay=2\0"

#define CFG_SDBOOTCOMMAND \
	"sdboot=setenv bootargs $bootargs_sd; ext4load mmc 0:1 $loadaddr boot/$bootfile; bootm $loadaddr$krncfg\0"

#define CFG_BOOTARGS_NAND \
	"bootargs_nand=earlycon=uart8250,mmio,0x21c0500 root=/dev/mtdblock0 rw noinitrd console=ttyS0,115200 rootfstype=jffs2\0"

#define CFG_NANDBOOTCOMMAND \
	"nandboot=setenv bootargs $bootargs_nand; sf probe; sf read $loadaddr 0x500000 0xb00000; bootm $loadaddr$krncfg\0"


#undef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS	"earlycon=uart8250,mmio,0x21c0500"


#define CONFIG_NFSBOOTCOMMAND			\
	"setenv bootargs $bootargs root=/dev/nfs rw "	\
	"nfsroot=$serverip:$rootpath,v3,tcp "		\
	"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname:$netdev:off " \
	"console=$consoledev,$baudrate $othbootargs;"	\
	"tftp $loadaddr ${tftppath}/$bootfile;"		\
	"bootm $loadaddr$krncfg"


#ifdef CONFIG_TFABOOT
#undef CONFIG_BOOTCOMMAND
#undef QSPI_NOR_BOOTCOMMAND
#undef SD_BOOTCOMMAND
#undef IFC_NAND_BOOTCOMMAND
#define QSPI_NOR_BOOTCOMMAND "mmc rescan; run sdboot; "
#define SD_BOOTCOMMAND "mmc rescan;run sdboot; "
#define IFC_NAND_BOOTCOMMAND "mmc rescan;run sdboot; "
#endif


    #if defined(CONFIG_CARRIER_CRX05)
        #define KRNCFG "krncfg=#mpxls1088"
        #define CONFIG_NETDEV "netdev=eth0"
    #endif
    #if defined(CONFIG_CARRIER_CRX06)
    #if defined(CONFIG_MPX1088_RCW_SDCARD_CRX06_2QSGMII)
        #define KRNCFG "krncfg=#mpxls1088_crx06_2qsgmii"
        #define CONFIG_NETDEV "netdev=eth0"
    #else
        #define KRNCFG "krncfg=#mpxls1088_crx06_2tsn"
        #define CONFIG_NETDEV "netdev=eth2"
    #endif
    #endif
    #define CONFIG_ROOTPATH		"/opt/fsl-image-networking-full-mpxls1088"

    #define TFTP_PATH				\
        "tftppath=MPXLS1088/2021.08\0"


#define CFG_RAMDISK_IMAGE \
    "ramdisk_bootfile=ramdisk_image.bin"

#define CFG_BOOTARGS_RAM \
	"bootargs_ram=earlycon=uart8250,mmio,0x21c0500 root=/dev/ram0 rw console=ttyS0,115200"

#define CONFIG_RAMBOOTCOMMAND \
	"setenv bootargs $bootargs_ram; tftp $loadaddr $tftppath/$ramdisk_bootfile; bootm $loadaddr"

#undef CONFIG_EXTRA_ENV_SETTINGS
#ifdef CONFIG_TFABOOT
#define CONFIG_EXTRA_ENV_SETTINGS	           \
	"hwconfig=fsl_ddr:bank_intlv=auto\0"       \
	"loadaddr=0xa0000000\0"                    \
	"kernel_nand=0x00200000\0"                 \
	"kernel_size=0x01700000\0"                 \
	"consoledev=ttyS0\0"                       \
	"fdt_high=0xa0000000\0"                    \
	"initrd_high=0xffffffffffffffff\0"         \
	"serverip=192.168.0.11\0"                  \
	"ipaddr=192.168.0.88\0"                    \
	"mcmemsize=0x70000000\0"                   \
	CONFIG_NETDEV"\0"  \
	KRNCFG"\0"  \
	CFG_RAMDISK_IMAGE"\0"	 \
	CFG_BOOTARGS_RAM"\0"	\
	TFTP_PATH	\
	CFG_BOOTARGS_SD  \
	CFG_BOOTARGS_NAND  \
	CFG_SDBOOTCOMMAND  \
	CFG_NANDBOOTCOMMAND

#else
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"BOARD=mpxls1088\0"			\
	"hwconfig=fsl_ddr:bank_intlv=auto\0"	\
	"ramdisk_addr=0x800000\0"		\
	"ramdisk_size=0x2000000\0"		\
	"fdt_high=0xa0000000\0"			\
	"initrd_high=0xffffffffffffffff\0"	\
	"fdt_addr=0x64f00000\0"			\
	"kernel_addr=0x1000000\0"		\
	"kernel_addr_sd=0x8000\0"		\
	"kernelhdr_addr_sd=0x4000\0"		\
	"kernel_start=0x580100000\0"		\
	"kernelheader_start=0x580800000\0"	\
	"scriptaddr=0x80000000\0"		\
	"scripthdraddr=0x80080000\0"		\
	"fdtheader_addr_r=0x80100000\0"		\
	"kernelheader_addr=0x800000\0"		\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"		\
	"kernelheader_size=0x40000\0"		\
	"fdt_addr_r=0x90000000\0"		\
	"load_addr=0xa0000000\0"		\
	"kernel_size=0x2800000\0"		\
	"kernel_size_sd=0x14000\0"		\
	"kernelhdr_size_sd=0x20\0"		\
	MC_INIT_CMD				\
	BOOTENV					\
	"boot_scripts=mpxls1088_boot.scr\0"	\
	"boot_script_hdr=hdr_mpxls1088_bs.out\0"	\
	"scan_dev_for_boot_part="		\
		"part list ${devtype} ${devnum} devplist; "	\
		"env exists devplist || setenv devplist 1; "	\
		"for distro_bootpart in ${devplist}; do "	\
			"if fstype ${devtype} "			\
				"${devnum}:${distro_bootpart} "	\
				"bootfstype; then "		\
				"run scan_dev_for_boot; "	\
			"fi; "					\
		"done\0"					\
	"boot_a_script="					\
		"load ${devtype} ${devnum}:${distro_bootpart} " \
		"${scriptaddr} ${prefix}${script}; "		\
	"env exists secureboot && load ${devtype} "		\
		"${devnum}:${distro_bootpart} "			\
		"${scripthdraddr} ${prefix}${boot_script_hdr} " \
		"&& esbc_validate ${scripthdraddr};"		\
		"source ${scriptaddr}\0"			\
	"installer=load mmc 0:2 $load_addr "			\
		"/flex_installer_arm64.itb; "			\
		"env exists mcinitcmd && run mcinitcmd && "	\
		"mmc read 0x80001000 0x6800 0x800;"		\
		"fsl_mc lazyapply dpl 0x80001000;"			\
		"bootm $load_addr#mpxls1088\0"			\
	"qspi_bootcmd=echo Trying load from qspi..;"		\
		"sf probe && sf read $load_addr "		\
		"$kernel_addr $kernel_size ; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; "\
		"bootm $load_addr#$BOARD\0"			\
		"sd_bootcmd=echo Trying load from sd card..;"		\
		"mmcinfo; mmc read $load_addr "			\
		"$kernel_addr_sd $kernel_size_sd ;"		\
		"env exists secureboot && mmc read $kernelheader_addr_r "\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "	\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$BOARD\0"
#endif /* CONFIG_TFABOOT */

/* MAC/PHY configuration */
#ifdef CONFIG_FSL_MC_ENET
#ifndef CONFIG_PHYLIB_10G
#define CONFIG_PHYLIB_10G
#endif
#ifndef CONFIG_PHY_GIGE
#define CONFIG_PHY_GIGE
#endif
#ifndef CONFIG_PHYLIB
#define CONFIG_PHYLIB
#endif

#ifndef CONFIG_PHY_MARVELL
#define CONFIG_PHY_MARVELL
#endif
#define CONFIG_PHY_VITESSE
#define AQ_PHY_ADDR1			0x00
#define AQR105_IRQ_MASK			0x00000004
#ifndef CONFIG_MII
#define CONFIG_MII
#endif
#ifdef CONFIG_CARRIER_CRX06
    #if defined(CONFIG_MPX1088_RCW_SDCARD_CRX06_2QSGMII)
        #define CONFIG_ETHPRIME		"DPMAC3@qsgmii"
    #else
        #define CONFIG_ETHPRIME		"DPMAC3@sgmii"
    #endif
#endif
#ifdef CONFIG_CARRIER_CRX05
#define CONFIG_ETHPRIME		"DPMAC4@rgmii-id"
#endif
#define CONFIG_RESET_PHY_R
#endif
#endif

/*  MMC  */
#ifndef CONFIG_MMC
#define CONFIG_MMC
#endif
#ifdef CONFIG_MMC
#ifndef CONFIG_CMD_MMC
#define CONFIG_CMD_MMC
#endif
#ifndef CONFIG_FSL_ESDHC
#define CONFIG_FSL_ESDHC
#endif
#define CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
#define CONFIG_ESDHC_DETECT_QUIRK	1
#define CFG_SYS_FSL_ERRATUM_ESDHC_A009620
#endif

/*
 * USB
 */
#define CONFIG_HAS_FSL_XHCI_USB
#ifndef CONFIG_USB_XHCI_FSL
#define CONFIG_USB_XHCI_FSL
#endif
#ifndef CONFIG_USB_XHCI_DWC3
#define CONFIG_USB_XHCI_DWC3
#endif
#define CONFIG_USB_MAX_CONTROLLER_COUNT		2
#ifndef CONFIG_CMD_USB
#define CONFIG_CMD_USB
#endif
#ifndef CONFIG_USB_STORAGE
#define CONFIG_USB_STORAGE
#endif

/*
#ifndef CONFIG_SPL_BUILD
#ifndef CONFIG_CMD_ESBC_VALIDATE
#define CONFIG_CMD_ESBC_VALIDATE
#endif
#endif
*/

#ifndef SPL_NO_ENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#endif

#include "microsys_common.h"

#include <asm/fsl_secure_boot.h>

#endif /* __MPXLS1088_H */
