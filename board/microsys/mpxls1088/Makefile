#
# Copyright 2017 NXP
# Copyright 2018 MicroSys Electronics GmbH
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += mpxls1088.o
obj-y += ddr.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_TARGET_MPXLS1088) += eth_mpxls1088.o
obj-$(CONFIG_ID_EEPROM)	+= ../../freescale/common/sys_eeprom.o
obj-$(CONFIG_CARRIER_CRX06) += ../crx06/mmd.o ../crx06/crx06.o ../common/eth_common.o
obj-$(CONFIG_CARRIER_CRX05) += ../common/eth_common.o
ifdef CONFIG_SECURE_BOOT
obj-$(CONFIG_SECURE_BOOT) += ../../freescale/common/fsl_validate.o ../../freescale/common/cmd_esbc_validate.o ../../freescale/common/fsl_chain_of_trust.o
endif
endif
