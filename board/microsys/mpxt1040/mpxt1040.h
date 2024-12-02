/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __MPXT1040_H__
#define __MPXT1040_H__

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, bd_t * bd);

#endif
