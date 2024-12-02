#
# Copyright 2018 Freescale Semiconductor
# Copyright (C) 2020 MicroSys Electronics GmbH
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += mpxlx2160a.o
obj-y += ddr.o
obj-$(CONFIG_TARGET_MPXLX2160A) += eth_mpxlx2160a.o do_ddr.o
obj-$(CONFIG_QSFP_EEPROM)	+= ../../freescale/common/qsfp_eeprom.o
obj-$(CONFIG_ID_EEPROM)	+= ../../freescale/common/sys_eeprom.o
obj-$(CONFIG_CARRIER_CRX08) += ../crx06/mmd.o
