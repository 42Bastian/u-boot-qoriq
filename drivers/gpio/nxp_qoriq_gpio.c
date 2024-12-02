// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 *
 * based on arch/powerpc/include/asm/mpc85xx_gpio.h, which is
 *
 * Copyright 2010 eXMeritus, A Boeing Company
 *
 * Copyright (C) 2020 MicroSys Electronics GmbH <kay.potthoff@microsys.de>
 *
 * based on arch/powerpc/include/asm/mpc8xxx_gpio.h
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/ioport.h>

typedef enum {
    BIG_ENDIAN,
    LITTLE_ENDIAN
} endianess_t;

typedef enum {
	QORIQ_GPIO_TYPE,
} gpio_type_t;



struct qoriq_gpio_plat {
    struct resource res;
    uint ngpios;
    endianess_t endianess;
};

struct qoriq_gpio_data {
	ulong type;
};

static inline u32 gpio_mask(uint gpio)
{
	return (1U << (31 - (gpio)));
}

static inline u32 qoriq_gpio_get_val(struct udevice *dev, u32 mask)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        return in_le32(&base->gpdat) & mask;
    default:
        return in_be32(&base->gpdat) & mask;
    }
}

static inline u32 qoriq_gpio_get_dir(struct udevice *dev, u32 mask)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        return in_le32(&base->gpdir) & mask;
    default:
        return in_be32(&base->gpdir) & mask;
    }
}

static inline void qoriq_gpio_set_in(struct udevice *dev, u32 gpios)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        clrbits_le32(&base->gpdat, gpios);
        clrbits_le32(&base->gpdir, gpios);
        break;
    default:
        clrbits_be32(&base->gpdat, gpios);
        clrbits_be32(&base->gpdir, gpios);
        break;
    }
}

static inline void qoriq_gpio_set_low(struct udevice *dev, u32 gpios)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        clrbits_le32(&base->gpdat, gpios);
        setbits_le32(&base->gpdir, gpios);
        break;
    default:
        clrbits_be32(&base->gpdat, gpios);
        setbits_be32(&base->gpdir, gpios);
        break;
    }
}

static inline void qoriq_gpio_set_high(struct udevice *dev, u32 gpios)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        setbits_le32(&base->gpdat, gpios);
        setbits_le32(&base->gpdir, gpios);
        break;
    default:
        setbits_be32(&base->gpdat, gpios);
        setbits_be32(&base->gpdir, gpios);
        break;
    }
}

static inline int qoriq_gpio_open_drain_val(struct udevice *dev, u32 mask)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        return in_le32(&base->gpodr) & mask;
    default:
        return in_be32(&base->gpodr) & mask;
    }
}

static inline void qoriq_gpio_open_drain_on(struct udevice *dev, u32
					      gpios)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        setbits_le32(&base->gpodr, gpios);
        break;
    default:
        setbits_be32(&base->gpodr, gpios);
        break;
    }
}

static inline void qoriq_gpio_open_drain_off(struct udevice *dev,
					       u32 gpios)
{
    struct qoriq_gpio_plat *plat = dev_get_plat(dev);
    struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);

    switch (plat->endianess) {
    case LITTLE_ENDIAN:
        clrbits_le32(&base->gpodr, gpios);
        break;
    default:
        clrbits_be32(&base->gpodr, gpios);
        break;
    }
}

static int qoriq_gpio_direction_input(struct udevice *dev, uint gpio)
{
	qoriq_gpio_set_in(dev, gpio_mask(gpio));
	return 0;
}

static int qoriq_gpio_set_value(struct udevice *dev, uint gpio, int value)
{
	if (value) {
		qoriq_gpio_set_high(dev, gpio_mask(gpio));
	} else {
		qoriq_gpio_set_low(dev, gpio_mask(gpio));
	}
	return 0;
}

static int qoriq_gpio_direction_output(struct udevice *dev, uint gpio,
					 int value)
{
	return qoriq_gpio_set_value(dev, gpio, value);
}

static int qoriq_gpio_get_value(struct udevice *dev, uint gpio)
{
	return !!qoriq_gpio_get_val(dev, gpio_mask(gpio));
}

static int qoriq_gpio_get_function(struct udevice *dev, uint gpio)
{
	int dir;

	dir = !!qoriq_gpio_get_dir(dev, gpio_mask(gpio));
	return dir ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int qoriq_gpio_of_to_plat(struct udevice *dev)
{
	struct qoriq_gpio_plat *plat = dev_get_plat(dev);

	ofnode_read_resource(dev_ofnode(dev), 0, &plat->res);

	plat->ngpios = dev_read_u32_default(dev, "ngpios", 32);

	if (ofnode_read_bool(dev_ofnode(dev), "little-endian"))
	    plat->endianess = LITTLE_ENDIAN;
	else
	    plat->endianess = BIG_ENDIAN;

	return 0;
}

static int qoriq_gpio_drv_to_priv(struct udevice *dev)
{
	struct qoriq_gpio_data *priv = dev_get_priv(dev);
	ulong driver_data = dev_get_driver_data(dev);

	priv->type = driver_data;

	return 0;
}

static int qoriq_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct qoriq_gpio_plat *plat = dev_get_plat(dev);
	struct ccsr_gpio *base = (struct ccsr_gpio *) (plat->res.start);
	ofnode node;
	char name[32], *str;

	qoriq_gpio_drv_to_priv(dev);

	node = dev_ofnode(dev);
	snprintf(name, sizeof(name), "%s-", ofnode_get_name(node));
	str = strdup(name);

	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = plat->ngpios;

	base->gpibe = ~0; // enable input buffer for all pins

	return 0;
}

static const struct dm_gpio_ops gpio_qoriq_ops = {
	.direction_input	= qoriq_gpio_direction_input,
	.direction_output	= qoriq_gpio_direction_output,
	.get_value		= qoriq_gpio_get_value,
	.set_value		= qoriq_gpio_set_value,
	.get_function		= qoriq_gpio_get_function,
};

static const struct udevice_id qoriq_gpio_ids[] = {
	{ .compatible = "fsl,qoriq-gpio", .data = QORIQ_GPIO_TYPE },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_qoriq) = {
	.name	= "gpio_qoriq",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_qoriq_ops,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_to_plat = qoriq_gpio_of_to_plat,
	.plat_auto = sizeof(struct qoriq_gpio_plat),
	.of_match = qoriq_gpio_ids,
#endif
	.probe	= qoriq_gpio_probe,
	.priv_auto  = sizeof(struct qoriq_gpio_data),
};
