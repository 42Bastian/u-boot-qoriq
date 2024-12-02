/* -*-C-*- */
/* SPDX-License-Identifier:    GPL-2.0+ */
/*
 * Copyright (C) 2020 MicroSys Electronics GmbH
 * Author: Kay Potthoff <kay.potthoff@microsys.de>
 *
 */

/*!
 * \addtogroup Miscellaneous TUSB8041
 * @{
 *
 * \file
 * SMBus/I2C driver for USB hub TUSB8041.
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>

#define DEV_STATUS_REG 0xf8
#define CFGACTIVE_BIT BIT(0)

static int tusb8041_probe(struct udevice *dev)
{
    uint8_t reg;

    /*
     * Check if the device is in configuration mode.
     * This is can be checked via bit 'cfgActive' in the
     * device status and command register. If so, the bit
     * needs to be cleared by writing 1 to it. Otherwise
     * the chip doesn't start working.
     *
     * Note that this assumes that the device has been configured
     * as SMBus device via strapping pin.
     */

    int rv = dm_i2c_read(dev, DEV_STATUS_REG, &reg, 1);

    if ((rv == 0) && (reg & CFGACTIVE_BIT)) {
        rv = dm_i2c_write(dev, DEV_STATUS_REG, &reg, 1);
    }
    else {
        puts("Error: cannot access TUSB8041!\n");
        rv = -EIO;
    }

    return rv;
}

static const struct udevice_id tusb8041_match[] = {
    { .compatible = "ti,tusb8041", },
    { },
};

U_BOOT_DRIVER(tusb8041) = {
    .name = "tusb8041",
    .id = UCLASS_I2C_GENERIC,
    .of_match = tusb8041_match,
    .probe = tusb8041_probe,
};

/*!@}*/

/* *INDENT-OFF* */
/******************************************************************************
 * Local Variables:
 * mode: C
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * kate: space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
 * vim: set expandtab filetype=c:
 * vi: set et tabstop=4 shiftwidth=4: */
