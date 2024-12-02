#
# Copyright 2013 Freescale Semiconductor, Inc.
#
# Copyright 2015 MicroSys GmbH
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y	+= mpxt1040.o
obj-y	+= eth.o
obj-$(CONFIG_PCI)	+= pci.o
obj-$(CONFIG_FSL_DIU_FB)+= diu.o
endif
obj-$(CONFIG_DEEP_SLEEP)+= mpc85xx_sleep.o
obj-$(CONFIG_FSL_DIU_CH7301)	+= diu_ch7033.o
obj-y	+= ddr.o
obj-y	+= law.o
obj-y	+= tlb.o
