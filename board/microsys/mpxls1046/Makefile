#
# Copyright 2017 MicroSys Electronics GmbH
#
# SPDX-License-Identifier:	GPL-2.0+
#

ccflags-$(CONFIG_CARRIER_CRX06) += -I$(srctree)/drivers/net/fm

obj-y += ddr.o
obj-y += mpxls1046.o
obj-$(CONFIG_SYS_DPAA_FMAN) += eth.o
obj-$(CONFIG_LAYERSCAPE_NS_ACCESS) += ../../freescale/common/ns_access.o
obj-$(CONFIG_CARRIER_CRX06) += ../crx06/mmd.o ../crx06/crx06.o
obj-$(CONFIG_ID_EEPROM)	+= ../../freescale/common/sys_eeprom.o
