/*
 * Copyright 2015 Freescale Semiconductor
 * (C) Copyright 2018 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MPXLS1043_H__
#define __MPXLS1043_H__

#include "ls1043a_common.h"

#define CONFIG_SYS_CLK_FREQ		100000000
#define CONFIG_DDR_CLK_FREQ		100000000

#define CONFIG_LAYERSCAPE_NS_ACCESS
#ifndef CONFIG_MISC_INIT_R
#define CONFIG_MISC_INIT_R
#endif

#define CONFIG_DIMM_SLOTS_PER_CTLR	1
/* Physical Memory Map */
#define CONFIG_CHIP_SELECTS_PER_CTRL	4
#undef CONFIG_NR_DRAM_BANKS
#define CONFIG_NR_DRAM_BANKS		2

#define CONFIG_SYS_DDR_RAW_TIMING

#define CONFIG_DDR_ECC
#define CONFIG_ECC_INIT_VIA_DDRCONTROLLER
#define CONFIG_MEM_INIT_VALUE           0xdeadbeef
#define CONFIG_FSL_DDR_BIST	/* enable built-in memory test */
#define CONFIG_FSL_DDR_INTERACTIVE	/* Interactive debugging */

#ifdef CONFIG_RAMBOOT_PBL
#define CONFIG_SYS_FSL_PBL_PBI board/microsys/mpxls1043/mpxls1043_pbi.cfg
#endif

#undef CONFIG_SYS_EXTRA_OPTIONS


#ifdef CONFIG_PCI
#ifndef CONFIG_CMD_PCI
#define CONFIG_CMD_PCI
#endif
#define CONFIG_PCI_BOOTDELAY       1000
#endif

#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* number of banks */
#define CONFIG_SYS_MAX_FLASH_SECT	1024	/* sectors per device */
#define CONFIG_SYS_FLASH_ERASE_TOUT	60000	/* Flash Erase Timeout (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (ms) */

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE_PHYS }

#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
#define CONFIG_SYS_WRITE_SWAPPED_DATA

/*
 * NAND Flash Definitions
 */
#define CONFIG_NAND_FSL_IFC

#define CONFIG_SYS_NAND_BASE		0x7e800000
#define CONFIG_SYS_NAND_BASE_PHYS	CONFIG_SYS_NAND_BASE

#define CONFIG_SYS_NAND_CSPR_EXT	(0x0)
#define CONFIG_SYS_NAND_CSPR	(CSPR_PHYS_ADDR(CONFIG_SYS_NAND_BASE_PHYS) \
				| CSPR_PORT_SIZE_8	\
				| CSPR_MSEL_NAND	\
				| CSPR_V)
#define CONFIG_SYS_NAND_AMASK	IFC_AMASK(64*1024)
#define CONFIG_SYS_NAND_CSOR	(CSOR_NAND_ECC_ENC_EN	/* ECC on encode */ \
				| CSOR_NAND_ECC_DEC_EN	/* ECC on decode */ \
				| CSOR_NAND_ECC_MODE_4	/* 4-bit ECC */ \
				| CSOR_NAND_RAL_3	/* RAL = 3 Bytes */ \
				| CSOR_NAND_PGS_2K	/* Page Size = 2K */ \
				| CSOR_NAND_SPRZ_64	/* Spare size = 64 */ \
				| CSOR_NAND_PB(64))	/* 64 Pages Per Block */

#define CONFIG_SYS_NAND_ONFI_DETECTION

#define CONFIG_SYS_NAND_FTIM0		(FTIM0_NAND_TCCST(0x7) | \
					FTIM0_NAND_TWP(0x18)   | \
					FTIM0_NAND_TWCHT(0x7) | \
					FTIM0_NAND_TWH(0xa))
#define CONFIG_SYS_NAND_FTIM1		(FTIM1_NAND_TADLE(0x32) | \
					FTIM1_NAND_TWBE(0x39)  | \
					FTIM1_NAND_TRR(0xe)   | \
					FTIM1_NAND_TRP(0x18))
#define CONFIG_SYS_NAND_FTIM2		(FTIM2_NAND_TRAD(0xf) | \
					FTIM2_NAND_TREH(0xa) | \
					FTIM2_NAND_TWHRE(0x1e))
#define CONFIG_SYS_NAND_FTIM3		0x0

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_MTD_NAND_VERIFY_WRITE

#ifndef CONFIG_CMD_NAND
#define CONFIG_CMD_NAND
#endif

#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)

/* IFC Timing Params */
#ifdef CONFIG_TFABOOT

#define CONFIG_SYS_CSPR1_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NAND_FTIM3
#else
#ifdef CONFIG_NAND_BOOT
#define CONFIG_SYS_CSPR0_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR0		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK0		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR0		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS0_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS0_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS0_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS0_FTIM3		CONFIG_SYS_NAND_FTIM3

#define CONFIG_SYS_CSPR1_EXT		CONFIG_SYS_NAND_CSPR_EXT
#define CONFIG_SYS_CSPR1		CONFIG_SYS_NAND_CSPR
#define CONFIG_SYS_AMASK1		CONFIG_SYS_NAND_AMASK
#define CONFIG_SYS_CSOR1		CONFIG_SYS_NAND_CSOR
#define CONFIG_SYS_CS1_FTIM0		CONFIG_SYS_NAND_FTIM0
#define CONFIG_SYS_CS1_FTIM1		CONFIG_SYS_NAND_FTIM1
#define CONFIG_SYS_CS1_FTIM2		CONFIG_SYS_NAND_FTIM2
#define CONFIG_SYS_CS1_FTIM3		CONFIG_SYS_NAND_FTIM3
#endif
#endif

/* EEPROM */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_I2C_EEPROM_NXID_MAC 10
#define CONFIG_SYS_EEPROM_BUS_NUM		0
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
#define I2C_RETIMER_ADDR			0x18

/*
 * Environment
 */

#ifndef CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_OVERWRITE
#endif

#ifdef CONFIG_TFABOOT
#define CONFIG_SYS_MMC_ENV_DEV		0

#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_OFFSET		0x500000
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x500000)
#define CONFIG_ENV_SECT_SIZE		0x20000
#else
#if defined(CONFIG_NAND_BOOT)
#define CONFIG_ENV_SIZE			0x2000
#define CONFIG_ENV_OFFSET		(24 * CONFIG_SYS_NAND_BLOCK_SIZE)
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_ENV_OFFSET		(3 * 1024 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			0x2000
#else
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0x300000)
#define CONFIG_ENV_SECT_SIZE		0x20000
#define CONFIG_ENV_SIZE			0x20000
#endif
#endif

/* FMan */
#ifdef CONFIG_SYS_DPAA_FMAN
#ifndef CONFIG_FMAN_ENET
#define CONFIG_FMAN_ENET
#endif
#ifndef CONFIG_PHYLIB
#define CONFIG_PHYLIB
#endif
#ifndef CONFIG_PHYLIB_10G
#define CONFIG_PHYLIB_10G
#endif
#ifndef CONFIG_PHY_GIGE
#define CONFIG_PHY_GIGE		/* Include GbE speed/duplex detection */
#endif
#ifndef CONFIG_PHY_MARVELL
#define CONFIG_PHY_MARVELL
#endif
#define AQR105_IRQ_MASK			0x40000000

#define SGMII_PHY1_ADDR			0x0
#define SGMII_PHY2_ADDR			0x2
#define	RGMII_PHY3_ADDR			0x3
#define FDT_SEQ_MACADDR_FROM_ENV
#define CONFIG_ETHPRIME			"FM1@DTSEC3"
#endif

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

/*
 * Define driver model for SPI:
 */
#if defined(CONFIG_FSL_QSPI) && !defined(CONFIG_DM_SPI)
#define CONFIG_DM_SPI
#endif

#ifdef CONFIG_CARRIER_CRX06
#ifndef CONFIG_FSL_DSPI
#define CONFIG_FSL_DSPI
#endif
#define FSL_DSPI_MAX_CHIPSELECT 4
#define CONFIG_SYS_DSPI_CTAR0 (DSPI_CTAR_TRSZ(0xf))
#define CONFIG_SYS_DSPI_CTAR1 (DSPI_CTAR_TRSZ(0xf) | DSPI_CTAR_CPHA)
#define CONFIG_SYS_DSPI_CTAR2 (DSPI_CTAR_TRSZ(0xf) | DSPI_CTAR_CPHA)
#define CONFIG_SYS_DSPI_CTAR3 (DSPI_CTAR_TRSZ(0xf))
#else
#undef CONFIG_FSL_DSPI
#endif

/* USB */
#ifndef SPL_NO_USB
#ifndef CONFIG_HAS_FSL_XHCI_USB
#define CONFIG_HAS_FSL_XHCI_USB
#endif
#ifdef CONFIG_HAS_FSL_XHCI_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT         3
#endif
#endif

/* SATA */
#ifndef CONFIG_LIBATA
#define CONFIG_LIBATA
#endif
#ifndef CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI
#endif
#ifndef CONFIG_DM_SCSI
#define CONFIG_DM_SCSI
#endif
#define CONFIG_SCSI_AHCI_PLAT
#ifndef CONFIG_SCSI
#define CONFIG_SCSI
#endif
#define CONFIG_SYS_SATA_MAX_DEVICE	2
#ifndef CONFIG_BOARD_LATE_INIT
#define CONFIG_BOARD_LATE_INIT
#endif
#define CONFIG_SYS_SATA				AHCI_BASE_ADDR

#define CONFIG_SYS_SCSI_MAX_SCSI_ID		1
#define CONFIG_SYS_SCSI_MAX_LUN			1
#define CONFIG_SYS_SCSI_MAX_DEVICE		(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)
#define SCSI_VEND_ID 0x1b4b
#define SCSI_DEV_ID  0x9170
#define CONFIG_SCSI_DEV_LIST {SCSI_VEND_ID, SCSI_DEV_ID}

#include "microsys_common.h"

#include <asm/fsl_secure_boot.h>

#endif /* __MPXLS1043_H__ */