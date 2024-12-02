/* SPDX-License-Identifier:    GPL-2.0+ */
/*
 * Copyright (C) 2019 MicroSys Electronics GmbH
 *
 */

#include <common.h>
#include <command.h>
#ifdef CONFIG_DM_RTC
#include <dm.h>
#endif
#include <rtc.h>
#include <i2c.h>

#ifdef CONFIG_DM_RTC

static int pcf85063_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
    uchar buf[7];

    buf[0] = bin2bcd(tm->tm_sec);
    buf[1] = bin2bcd(tm->tm_min);
    buf[2] = bin2bcd(tm->tm_hour);
    buf[3] = bin2bcd(tm->tm_mday);
    buf[5] = bin2bcd(tm->tm_mon);
    buf[6] = bin2bcd(tm->tm_year - 2000);
    buf[4] = bin2bcd(tm->tm_wday);

    return dm_i2c_write(dev, 0x04, buf, 7);
}

static int pcf85063_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
    uchar buf[7];
    int rv;

    rv = dm_i2c_read(dev, 0x04, buf, 7);

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

static int pcf85063_rtc_reset(struct udevice *dev)
{
    uchar buf[1] = { 0x58 };

    return dm_i2c_write(dev, 0x00, buf, 1);
}

static int pcf85063_probe(struct udevice *dev)
{
	return 0;
}

static const struct rtc_ops pcf85063_rtc_ops = {
	.get = pcf85063_rtc_get,
	.set = pcf85063_rtc_set,
	.reset = pcf85063_rtc_reset,
};

static const struct udevice_id pcf85063_rtc_ids[] = {
	{ .compatible = "nxp,pcf85063" },
	{ }
};

U_BOOT_DRIVER(rtc_pcf85063) = {
	.name	  = "rtc-pcf85063",
	.id	      = UCLASS_RTC,
	.probe	  = pcf85063_probe,
	.of_match = pcf85063_rtc_ids,
	.ops	  = &pcf85063_rtc_ops,
};
#else

static uchar rtc_read(uchar reg)
{
	return i2c_reg_read(CONFIG_SYS_I2C_RTC_ADDR, reg);
}

static void rtc_write(uchar reg, uchar val)
{
	i2c_reg_write(CONFIG_SYS_I2C_RTC_ADDR, reg, val);
}

int rtc_get(struct rtc_time *tm)
{
	uchar buf[7];
	int rv;

	rv = i2c_read(CONFIG_SYS_I2C_RTC_ADDR, 0x04, 1, buf, 7);

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

int rtc_set(struct rtc_time *tm)
{
	uchar buf[7];

	buf[0] = bin2bcd(tm->tm_sec);
	buf[1] = bin2bcd(tm->tm_min);
	buf[2] = bin2bcd(tm->tm_hour);
	buf[3] = bin2bcd(tm->tm_mday);
	buf[5] = bin2bcd(tm->tm_mon);
	buf[6] = bin2bcd(tm->tm_year - 2000);
	buf[4] = bin2bcd(tm->tm_wday);

	return i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 0x04, 1, buf, 7);
}

void rtc_reset(void)
{
	rtc_write(0x00, 0x58);
}

void rtc_init(void)
{
	uchar sec;

	// check for oscillator stop:
	sec = rtc_read(0x04);
	if (sec & (1<<7)) {
		// clear oscillator flag:
		rtc_write(0x04, sec & 0x7f);
	}

	rtc_write(0x00, 0x00);
}

#endif

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
