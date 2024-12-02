/* -*-C-*- */
/* SPDX-License-Identifier:    GPL-2.0+ */
/*
 * Copyright (C) 2020 MicroSys Electronics GmbH
 * Author: Kay Potthoff <kay.potthoff@microsys.de>
 *
 */

#include <common.h>
#include <command.h>
#ifdef CONFIG_DM_RTC
#include <dm.h>
#endif
#include <rtc.h>
#include <i2c.h>

#ifndef CONFIG_DM_RTC
#error "This driver needs CONFIG_DM_RTC enabled!"
#endif

static int rv8564_stop_bit(struct udevice *dev, bool stop)
{
    uchar buf[1] = { 0x00 };
    int rv = 0;

    rv = dm_i2c_read(dev, 0x00, buf, 1);

    if (rv != 0) return rv;

    if (stop)
        buf[0] |= BIT(5);
    else
        buf[0] &= ~BIT(5);

    rv = dm_i2c_write(dev, 0x00, buf, 1);

    return rv;
}

static int rv8564_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
    uchar buf[7];
    int rv = 0;

    buf[0] = bin2bcd(tm->tm_sec);
    buf[1] = bin2bcd(tm->tm_min);
    buf[2] = bin2bcd(tm->tm_hour);

    buf[3] = bin2bcd(tm->tm_mday);
    buf[4] = bin2bcd(tm->tm_wday);

    buf[5] = bin2bcd(tm->tm_mon);
    buf[6] = bin2bcd(tm->tm_year - 2000);

    rv = rv8564_stop_bit(dev, true);

    if (rv != 0) return rv;

    rv = dm_i2c_write(dev, 0x02, buf, 7);

    if (rv != 0) return rv;

    rv = rv8564_stop_bit(dev, false);

    return rv;
}

static int rv8564_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
    uchar buf[7];
    int rv;

    rv = dm_i2c_read(dev, 0x02, buf, 7);

    if (rv != 0) return rv;

    tm->tm_sec  = bcd2bin(buf[0] & 0x7f);
    tm->tm_min  = bcd2bin(buf[1] & 0x7f);
    tm->tm_hour = bcd2bin(buf[2] & 0x3f);

    tm->tm_mday = bcd2bin(buf[3] & 0x3f);
    tm->tm_mon  = bcd2bin(buf[5] & 0x1f);
    tm->tm_year = bcd2bin(buf[6]) + 2000;

    tm->tm_wday = bcd2bin(buf[4] & 0x07);
    tm->tm_yday = 0;
    tm->tm_isdst= 0;

    return 0;
}

static int rv8564_rtc_reset(struct udevice *dev)
{
    return 0;
}

static int rv8564_probe(struct udevice *dev)
{
    rv8564_stop_bit(dev, false);

    return 0;
}

static const struct rtc_ops rv8564_rtc_ops = {
    .get = rv8564_rtc_get,
    .set = rv8564_rtc_set,
    .reset = rv8564_rtc_reset,
};

static const struct udevice_id rv8564_rtc_ids[] = {
    { .compatible = "microcrystal,rv8564" },
    { }
};

U_BOOT_DRIVER(rtc_rv8564) = {
    .name     = "rtc-rv8564",
    .id       = UCLASS_RTC,
    .probe    = rv8564_probe,
    .of_match = rv8564_rtc_ids,
    .ops      = &rv8564_rtc_ops,
};

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
