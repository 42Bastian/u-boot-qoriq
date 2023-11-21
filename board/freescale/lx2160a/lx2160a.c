// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 */

#include <common.h>
#include <clock_legacy.h>
#include <dm.h>
#include <init.h>
#include <asm/global_data.h>
#include <dm/platform_data/serial_pl01x.h>
#include <i2c.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <linux/bitops.h>
#include <linux/libfdt.h>
#include <linux/delay.h>
#include <fsl-mc/fsl_mc.h>
#include <env_internal.h>
#include <efi_loader.h>
#include <asm/arch/mmu.h>
#include <hwconfig.h>
#include <asm/arch/clock.h>
#include <asm/arch/config.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include "../common/i2c_mux.h"

#include "../common/qixis.h"
#include "../common/vid.h"
#include <fsl_immap.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <asm/gpio.h>
#include "lx2160a.h"
#include "../common/qsfp_eeprom.h"
#include "../common/i2c_mux.h"
#include <cpu_func.h>

#ifdef CONFIG_EMC2305
#include "../common/emc2305.h"
#endif

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)  || \
    defined(CONFIG_TARGET_LA1238RDB)
#define CFG_MUX_I2C_SDHC(reg, value)		((reg & 0x3f) | value)
#define SET_CFG_MUX1_SDHC1_SDHC(reg)		(reg & 0x3f)
#define SET_CFG_MUX2_SDHC1_SPI(reg, value)	((reg & 0xcf) | value)
#define SET_CFG_MUX3_SDHC1_SPI(reg, value)	((reg & 0xf8) | value)
#define SET_CFG_MUX_SDHC2_DSPI(reg, value)	((reg & 0xf8) | value)
#define SET_CFG_MUX1_SDHC1_DSPI(reg, value)	((reg & 0x3f) | value)
#define SDHC1_BASE_PMUX_DSPI			2
#define SDHC2_BASE_PMUX_DSPI			2
#define IIC5_PMUX_SPI3				3
#endif /* CONFIG_TARGET_LX2160AQDS or CONFIG_TARGET_LX2162AQDS */

DECLARE_GLOBAL_DATA_PTR;

static struct pl01x_serial_plat serial0 = {
#if CONFIG_CONS_INDEX == 0
	.base = CONFIG_SYS_SERIAL0,
#elif CONFIG_CONS_INDEX == 1
	.base = CONFIG_SYS_SERIAL1,
#ifndef CONFIG_TARGET_LA1224RDB
	CONFIG_CONS_INDEX == 2
	.base = CONFIG_SYS_SERIAL2,
#elif CONFIG_CONS_INDEX == 3
	.base = CONFIG_SYS_SERIAL3,
#endif /* CONFIG_TARGET_LA1224RDB */
#else
#error "Unsupported console index value."
#endif
	.type = TYPE_PL011,
};

U_BOOT_DRVINFO(nxp_serial0) = {
	.name = "serial_pl01x",
	.plat = &serial0,
};

static struct pl01x_serial_plat serial1 = {
	.base = CONFIG_SYS_SERIAL1,
	.type = TYPE_PL011,
};

U_BOOT_DRVINFO(nxp_serial1) = {
	.name = "serial_pl01x",
	.plat = &serial1,
};

#if defined(CONFIG_TARGET_LA1238RDB)
static struct pl01x_serial_plat serial2 = {
	.base = CONFIG_SYS_SERIAL2,
	.type = TYPE_PL011,
};

U_BOOT_DRVINFO(nxp_serial2) = {
	.name = "serial_pl01x",
	.plat = &serial2,
};
#endif

#ifdef CONFIG_TARGET_LA1224RDB
static struct pl01x_serial_plat serial2 = {
	.base = CONFIG_SYS_SERIAL2,
	.type = TYPE_PL011,
};

U_BOOT_DRVINFO(nxp_serial2) = {
	.name = "serial_pl01x",
	.plat = &serial2,
};

static struct pl01x_serial_plat serial3 = {
	.base = CONFIG_SYS_SERIAL3,
	.type = TYPE_PL011,
};

U_BOOT_DRVINFO(nxp_serial3) = {
	.name = "serial_pl01x",
	.plat = &serial3,
};
#endif /* CONFIG_TARGET_LA1224RDB */

#if defined(CONFIG_TARGET_LA1238RDB)
static inline uint32_t get_board_version(void);
#endif

// Get the Board revision number of LA1224RDB Board
#if defined(CONFIG_TARGET_LA1224RDB)
int board_revision_num(void)
{
	struct udevice *dev;
	int ret;
	u8  rev_port_val;
	u32 conf_reg;
	u32 input_reg;
	u8  all_input = 0xff;

	/*
	 * LA1224RDB Rev C board has PCAL6524 i2c io expander and Rev A/B boards
	 * have PCAL6416A i2c io expander, these both don't have a compatible
	 * register map.
	 *
	 * I2C io expander's address is 0x22 on rev c board and 0x20 on rev a/b
	 * boards. 1st try to search 0x22, if it is available then access i2c
	 * io expander as per PCAL6524 register map, if 0x22 is not found
	 * then try to find 0x20 and access i2c io expander as per PCAL6416A
	 * register map.
	 */
	ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR_PRI_REVC, 1, &dev);
	if (ret) {
		ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR_PRI_REVAB, 1,
					      &dev);
		if (ret) {
			printf("%s: I2C io expander not detected\n", __func__);
			return ret;
		}
		conf_reg = IO_EXAPNDER_CONF_REG_REVAB;
		input_reg = IO_EXAPNDER_INPUT_REG_REVAB;
	} else {
		conf_reg = IO_EXAPNDER_CONF_REG_REVC;
		input_reg = IO_EXAPNDER_INPUT_REG_REVC;
	}

	ret = dm_i2c_write(dev, conf_reg, &all_input, 1);
	if (!ret) {
		ret = dm_i2c_read(dev, input_reg, &rev_port_val, 1);
		if (ret) {
			printf("%s: rev port read failed\n", __func__);
			return ret;
		}
	} else {
		printf("%s: failed to enable rev port as input\n", __func__);
		return ret;
	}

	return ((rev_port_val >> BOARD_REV_SHIFT_MASK) & BOARD_REV_MASK);
}

#endif /* CONFIG_TARGET_LA1224RDB */

#ifdef NOT_NEEDED_NOW
int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

#ifndef CONFIG_DM_I2C
	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
#else
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(0, I2C_MUX_PCA_ADDR_PRI, 1, &dev);
	if (ret) {
		printf("PCA: failed to get udev : %d\n", ret);
		return ret;
	} else {
		ret = dm_i2c_write(dev, 0, &ch, 1);
	}
#endif
	if (ret) {
		printf("PCA: failed to select channel : %d\n", ret);
		return ret;
	}

	return 0;
}
#endif

#ifdef NOT_NEEDED_NOW
#if !defined(CONFIG_TARGET_LA1238RDB)
int select_i2c_ch_pca9547_sec(u8 ch)
{
	int ret;

#ifndef CONFIG_DM_I2C
	ret = i2c_write(I2C_MUX_PCA_ADDR_SEC, 0, 1, &ch, 1);
#else
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(0, I2C_MUX_PCA_ADDR_SEC, 1, &dev);
	if (!ret)
		ret = dm_i2c_write(dev, 0, &ch, 1);
#endif
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}
#endif
#endif

static void uart_get_clock(void)
{
	serial0.clock = get_serial_clock();
	serial1.clock = get_serial_clock();
#if defined(CONFIG_TARGET_LA1238RDB)
	serial2.clock = get_serial_clock();
#endif
#ifdef CONFIG_TARGET_LA1224RDB
	serial2.clock = get_serial_clock();
	serial2.clock = get_serial_clock();
	serial3.clock = get_serial_clock();
#endif
}

int board_early_init_f(void)
{
#if defined(CONFIG_SYS_I2C_EARLY_INIT) && defined(CONFIG_SPL_BUILD)
	i2c_early_init_f();
#endif
	/* get required clock for UART IP */
	uart_get_clock();

#ifdef CONFIG_EMC2305
	select_i2c_ch_pca9547(I2C_MUX_CH_EMC2305, 0);
	emc2305_init(I2C_EMC2305_ADDR);
	set_fan_speed(I2C_EMC2305_PWM, I2C_EMC2305_ADDR);
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);
#endif

	fsl_lsch3_early_init_f();
	return 0;
}

#ifdef CONFIG_OF_BOARD_FIXUP
int board_fix_fdt(void *fdt)
{
	char *reg_names, *reg_name;
	int names_len, old_name_len, new_name_len, remaining_names_len;
	struct str_map {
		char *old_str;
		char *new_str;
	} reg_names_map[] = {
		{ "ccsr", "dbi" },
		{ "pf_ctrl", "ctrl" }
	};
	int off = -1, i = 0;

	if (IS_SVR_REV(get_svr(), 1, 0))
		return 0;

	fdt_for_each_node_by_compatible(off, fdt, -1, "fsl,lx2160a-pcie") {
		fdt_setprop(fdt, off, "compatible", "fsl,ls-pcie",
			    strlen("fsl,ls-pcie") + 1);

		reg_names = (char *)fdt_getprop(fdt, off, "reg-names",
						&names_len);
		if (!reg_names)
			continue;

		reg_name = reg_names;
		remaining_names_len = names_len - (reg_name - reg_names);
		i = 0;
		while ((i < ARRAY_SIZE(reg_names_map)) && remaining_names_len) {
			old_name_len = strlen(reg_names_map[i].old_str);
			new_name_len = strlen(reg_names_map[i].new_str);
			if (memcmp(reg_name, reg_names_map[i].old_str,
				   old_name_len) == 0) {
				/* first only leave required bytes for new_str
				 * and copy rest of the string after it
				 */
				memcpy(reg_name + new_name_len,
				       reg_name + old_name_len,
				       remaining_names_len - old_name_len);
				/* Now copy new_str */
				memcpy(reg_name, reg_names_map[i].new_str,
				       new_name_len);
				names_len -= old_name_len;
				names_len += new_name_len;
				i++;
			}

			reg_name = memchr(reg_name, '\0', remaining_names_len);
			if (!reg_name)
				break;

			reg_name += 1;

			remaining_names_len = names_len -
					      (reg_name - reg_names);
		}

		fdt_setprop(fdt, off, "reg-names", reg_names, names_len);
	}

	/* Fixup u-boot's DTS in case this is a revC board and
	 * we're using DM_ETH.
	 */
#if (defined(CONFIG_TARGET_LX2160ARDB) && defined(CONFIG_DM_ETH))
		fdt_fixup_board_phy_revc(fdt);
#endif
	return 0;
}
#endif

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
void esdhc_dspi_status_fixup(void *blob)
{
	const char esdhc0_path[] = "/soc/esdhc@2140000";
	const char esdhc1_path[] = "/soc/esdhc@2150000";
	const char dspi0_path[] = "/soc/spi@2100000";
	const char dspi1_path[] = "/soc/spi@2110000";
	const char dspi2_path[] = "/soc/spi@2120000";

	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 sdhc1_base_pmux;
	u32 sdhc2_base_pmux;
	u32 iic5_pmux;

	/* Check RCW field sdhc1_base_pmux to enable/disable
	 * esdhc0/dspi0 DT node
	 */
	sdhc1_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_SDHC1_BASE_PMUX_MASK;
	sdhc1_base_pmux >>= FSL_CHASSIS3_SDHC1_BASE_PMUX_SHIFT;

	if (sdhc1_base_pmux == SDHC1_BASE_PMUX_DSPI) {
		do_fixup_by_path(blob, dspi0_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, esdhc0_path, "status", "disabled",
				 sizeof("disabled"), 1);
	} else {
		do_fixup_by_path(blob, esdhc0_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, dspi0_path, "status", "disabled",
				 sizeof("disabled"), 1);
	}

	/* Check RCW field sdhc2_base_pmux to enable/disable
	 * esdhc1/dspi1 DT node
	 */
	sdhc2_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR13_REGSR - 1])
		& FSL_CHASSIS3_SDHC2_BASE_PMUX_MASK;
	sdhc2_base_pmux >>= FSL_CHASSIS3_SDHC2_BASE_PMUX_SHIFT;

	if (sdhc2_base_pmux == SDHC2_BASE_PMUX_DSPI) {
		do_fixup_by_path(blob, dspi1_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, esdhc1_path, "status", "disabled",
				 sizeof("disabled"), 1);
	} else {
		do_fixup_by_path(blob, esdhc1_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, dspi1_path, "status", "disabled",
				 sizeof("disabled"), 1);
	}

	/* Check RCW field IIC5 to enable dspi2 DT node */
	iic5_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_IIC5_PMUX_MASK;
	iic5_pmux >>= FSL_CHASSIS3_IIC5_PMUX_SHIFT;

	if (iic5_pmux == IIC5_PMUX_SPI3)
		do_fixup_by_path(blob, dspi2_path, "status", "okay",
				 sizeof("okay"), 1);
	else
		do_fixup_by_path(blob, dspi2_path, "status", "disabled",
				 sizeof("disabled"), 1);
}
#endif

int esdhc_status_fixup(void *blob, const char *compat)
{
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	/* Enable esdhc and dspi DT nodes based on RCW fields */
	esdhc_dspi_status_fixup(blob);
#elif defined(CONFIG_TARGET_LA1224RDB)
	if (board_revision_num() == REVA) {
		static const char esdhc_path[] = "/soc/esdhc@2140000";

		printf("####Disabling ESDHC on LA1224RDB Rev A platform: %s\n",
		       esdhc_path);
		do_fixup_by_path(blob, esdhc_path, "status", "disabled",
				 sizeof("disabled"), 1);
	}
#else
	/* Enable both esdhc DT nodes for LX2160ARDB LA1238RDB and LA1224RDB */
	do_fixup_by_compat(blob, compat, "status", "okay",
			   sizeof("okay"), 1);
#endif
	return 0;
}

#if defined(CONFIG_VID)
int i2c_multiplexer_select_vid_channel(u8 channel)
{
// Software change for Erratum E-00011: VID Adjustment
// RevA VID is fixed
#if defined(CONFIG_TARGET_LA1224RDB)
	if (board_revision_num() == REVA)
		return 0;
#endif /*CONFIG_TARGET_LA1224RDB */
	return select_i2c_ch_pca9547(channel, 0);
}

int init_func_vid(void)
{
// Software change for Erratum E-00011: VID Adjustment
// RevA VID is fixed
#if defined(CONFIG_TARGET_LA1224RDB)
	if (board_revision_num() == REVA)
		return 0;
#endif /* CONFIG_TARGET_LA1224RDB */
	int set_vid;

	if (IS_SVR_REV(get_svr(), 1, 0))
		set_vid = adjust_vdd(800);
	else
		set_vid = adjust_vdd(0);

	if (set_vid < 0)
		printf("core voltage not adjusted\n");

	return 0;
}
#endif
#if defined(CONFIG_TARGET_LA1238RDB)

#define LS_GPIO_NUMBER(port, gpio) ((((port) - 1) * 32) + ((gpio) & 31))

static void toggle_la1238_jtag_tck_gpio(void)
{
	int i;

	gpio_request(LS_GPIO_NUMBER(4, 3), "JTAG_TCK_TMS_MUX");
	gpio_direction_output(LS_GPIO_NUMBER(4, 3), 0);

	for (i = 0; i < 10; i++) {
		gpio_set_value(LS_GPIO_NUMBER(4, 3), 0);
		mdelay(1);
		gpio_set_value(LS_GPIO_NUMBER(4, 3), 1);
		mdelay(1);
	}
}

static void la1238rdb_gpio_init(void)
{
	gpio_request(LS_GPIO_NUMBER(4, 7), "SW_LEA_6T_RST_B");
	gpio_request(LS_GPIO_NUMBER(4, 6), "SW_MODEM_PORST_B");
	gpio_request(LS_GPIO_NUMBER(4, 8), "SW_AQR113_RST_B");
	gpio_request(LS_GPIO_NUMBER(4, 9), "SW_NLM_RST_B");
	gpio_request(LS_GPIO_NUMBER(4, 0), "SW_MODEM_WARM_RST_B");
	gpio_request(LS_GPIO_NUMBER(3, 15), "SW_SI5518_RST_B");
	gpio_request(LS_GPIO_NUMBER(1, 23), "STATUS_LED");
	gpio_request(LS_GPIO_NUMBER(4, 1), "CPU_GPIO_LED2_B");
	gpio_request(LS_GPIO_NUMBER(4, 2), "CPU_GPIO_LED1_B");
	gpio_request(LS_GPIO_NUMBER(4, 10), "GPIO_FUSE_PROG");
	gpio_request(LS_GPIO_NUMBER(4, 25), "CFG_MUX_1PPS_S1");
	gpio_request(LS_GPIO_NUMBER(4, 26), "CFG_MUX_1PPS_S2");
	gpio_request(LS_GPIO_NUMBER(1, 22), "LEA_6T_EXTINT0");
	gpio_request(LS_GPIO_NUMBER(4, 5), "GPIO_NLM_CPU");
	gpio_request(LS_GPIO_NUMBER(1, 26), "SI5518_GPIO1");
	gpio_request(LS_GPIO_NUMBER(1, 27), "SI5518_GPIO2");
	gpio_request(LS_GPIO_NUMBER(4, 4), "CPU_GPIO_TO_LA1238_1");
	gpio_request(LS_GPIO_NUMBER(4, 11), "CPU_GPIO_TO_LA1238_2");
	gpio_request(LS_GPIO_NUMBER(1, 15), "unused_mux1");
	gpio_request(LS_GPIO_NUMBER(1, 16), "unused_mux2");
	gpio_request(LS_GPIO_NUMBER(1, 17), "unused_mux3");
	gpio_request(LS_GPIO_NUMBER(1, 18), "unused_mux4");
	gpio_request(LS_GPIO_NUMBER(1, 19), "unused_mux5");
	gpio_request(LS_GPIO_NUMBER(1, 20), "unused_mux6");
	gpio_request(LS_GPIO_NUMBER(1, 21), "unused_mux7");
	gpio_request(LS_GPIO_NUMBER(1, 5), "unused_mux8");
	gpio_request(LS_GPIO_NUMBER(1, 4), "unused_mux9");

	gpio_direction_output(LS_GPIO_NUMBER(4, 7), 1);
	gpio_direction_output(LS_GPIO_NUMBER(4, 6), 1);
	gpio_direction_output(LS_GPIO_NUMBER(4, 8), 1);
	gpio_direction_output(LS_GPIO_NUMBER(4, 9), 1);
	gpio_direction_output(LS_GPIO_NUMBER(4, 0), 1);
	gpio_direction_output(LS_GPIO_NUMBER(3, 15), 1);
	gpio_direction_output(LS_GPIO_NUMBER(1, 23), 0);
	gpio_direction_output(LS_GPIO_NUMBER(4, 1), 0);
	gpio_direction_output(LS_GPIO_NUMBER(4, 2), 0);
	gpio_direction_output(LS_GPIO_NUMBER(4, 10), 1);
	gpio_direction_output(LS_GPIO_NUMBER(4, 25), 1);
	gpio_direction_output(LS_GPIO_NUMBER(4, 26), 1);
	gpio_direction_output(LS_GPIO_NUMBER(1, 22), 1);

	gpio_direction_input(LS_GPIO_NUMBER(4, 5));
	gpio_direction_input(LS_GPIO_NUMBER(1, 26));
	gpio_direction_input(LS_GPIO_NUMBER(1, 27));
	gpio_direction_input(LS_GPIO_NUMBER(4, 4));
	gpio_direction_input(LS_GPIO_NUMBER(4, 11));

	gpio_direction_output(LS_GPIO_NUMBER(1, 15), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 16), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 17), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 18), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 19), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 20), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 21), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 5), 0);
	gpio_direction_output(LS_GPIO_NUMBER(1, 4), 0);

	u32 rev = get_board_version();

	if (rev == REVA) {
		gpio_request(LS_GPIO_NUMBER(3, 12), "SI5518_GPIO3");
		gpio_request(LS_GPIO_NUMBER(3, 16), "unused_mux10");
		gpio_direction_input(LS_GPIO_NUMBER(3, 12));
		gpio_direction_output(LS_GPIO_NUMBER(3, 16), 0);
	} else if (rev == REVB) {
		gpio_request(LS_GPIO_NUMBER(3, 12), "CPU_EVT0_B");
		gpio_request(LS_GPIO_NUMBER(3, 16), "SI5518_GPIO3");
		gpio_direction_input(LS_GPIO_NUMBER(3, 12));
		gpio_direction_input(LS_GPIO_NUMBER(3, 16));
	} else {
		printf("Unknown - Board rev %x\n", rev);
	}
}

static inline uint32_t get_board_version(void)
{
	int val_13, val_14;

	gpio_request(LS_GPIO_NUMBER(3, 13), "gpio_ver_1");
	gpio_request(LS_GPIO_NUMBER(3, 14), "gpio_ver_2");

	gpio_direction_input(LS_GPIO_NUMBER(3, 13));
	gpio_direction_input(LS_GPIO_NUMBER(3, 14));

	val_13 = gpio_get_value(LS_GPIO_NUMBER(3, 13));
	val_14 = gpio_get_value(LS_GPIO_NUMBER(3, 14));

	return ((val_14 << 1) | val_13);
}

int checkboard(void)
{
	enum boot_src src = get_boot_src();
	u32 rev = get_board_version();
	int mode;
	char buf[64];

	cpu_name(buf);
	printf("CPU: %s\n", buf);

	mode = gpio_get_value(LS_GPIO_NUMBER(4, 24));
	switch (mode) {
	case 0:
		puts("Board: LA1234-RDB, ");
		break;
	case 1:
		puts("Board: LA1238-RDB, ");
		break;
	}

	switch (rev) {
	case REVA:
		puts("Rev: A, boot from ");
		break;
	case REVB:
		puts("Rev: B, boot from ");
		break;
	default:
		puts("Rev: Unknown, boot from ");
		break;
	}

	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD\n");
	else if (src == BOOT_SOURCE_SD_MMC2)
		puts("eMMC\n");
	else if (src == BOOT_SOURCE_XSPI_NOR)
		puts("FlexSPI NOR\n");
	else
		puts("Invalid source\n");

	puts("SERDES1 Reference Clock = 156.25\n");
	puts("SERDES2 Reference Clock1 = 100MHz Clock2 = 100MHz\n");

	return 0;
}

void fdt_fixup_board_model(void *blob)
{
	gpio_request(LS_GPIO_NUMBER(4, 24), "board_rev");
	gpio_direction_input(LS_GPIO_NUMBER(4, 24));
	if (gpio_get_value(LS_GPIO_NUMBER(4, 24)) == 0)
		do_fixup_by_path(blob, "/", "model",
				 "NXP Layerscape LA1234-RDB",
				 sizeof("NXP Layerscape LA1234-RDB"), 1);
}

#if defined(CONFIG_DISPLAY_BOARDINFO)
int show_board_info(void)
{
#ifdef CONFIG_OF_CONTROL
	gpio_request(LS_GPIO_NUMBER(4, 24), "board_rev");
	gpio_direction_input(LS_GPIO_NUMBER(4, 24));
	if (gpio_get_value(LS_GPIO_NUMBER(4, 24)) == 0)
		printf("Model: NXP Layerscape LA1234-RDB\n");
	else
		printf("Model: %s\n",
		       (char *)fdt_getprop(gd->fdt_blob, 0, "model", NULL));
#endif
	return checkboard();
}
#endif
#elif defined(CONFIG_TARGET_LA1224RDB)
int checkboard(void)
{
	enum boot_src src = get_boot_src();
	struct udevice *dev;
	int board_rev;
	u32 i2c_chip_addr;
	u32 conf_reg;
	u32 output_reg;
	/* Enable UART bit as output of IO expander */
	u8 conf = 0xFB, val = 0;
	u8  rev_port_val;

	printf("Board: LA1224-RDB, ");

	board_rev = board_revision_num();

	printf("Board version: %c, boot from ", board_rev + 'A');

	switch (board_rev + 'A') {
	case 'A':
	case 'B':
			i2c_chip_addr = I2C_IO_EXP_ADDR_PRI_REVAB;
			conf_reg = IO_EXAPNDER_CONF_REG_REVAB;
			output_reg = IO_EXAPNDER_OUTPUT_REG_REVAB;
	break;
	case 'C':
			i2c_chip_addr = I2C_IO_EXP_ADDR_PRI_REVC;
			conf_reg = IO_EXAPNDER_CONF_REG_REVC;
			output_reg = IO_EXAPNDER_OUTPUT_REG_REVC;
			break;
	default:
			printf("Invalid board revision\n");
			return -1;
	};

	if (!(i2c_get_chip_for_busnum(0, i2c_chip_addr, 1, &dev))) {
		if (!(dm_i2c_write(dev, conf_reg, &conf, 1))) {
			if (!dm_i2c_read(dev, output_reg, &val, 1)) {
				/* enable LX2_UART_EN of IO expander */
				val = val | UART_EN_MASK;
				if ((dm_i2c_write(dev, output_reg, &val, 1))) {
					printf("write fail on IO expander\n");
					return -1;
				}
			} else {
				printf("read fail of IO expander\n");
				return -1;
			}
		} else {
			printf("fail to write config register of expander\n");
			return -1;
		}
	} else {
		printf("failed to select IO expander\n");
		return -1;
	}

	if (src == BOOT_SOURCE_SD_MMC) {
		puts("SD\n");
	} else if (src == BOOT_SOURCE_XSPI_NOR) {
		puts("FlexSPI NOR");
		if (board_revision_num() == REVC) {
			conf_reg = IO_EXAPNDER_P0_INPUT_REG_REVC;
			dm_i2c_read(dev, conf_reg, &rev_port_val, 1);
			if (rev_port_val & (1 << 4))
				puts(" DEV#1\n");
			else
				puts(" DEV#0\n");
		}
	} else if (src == BOOT_SOURCE_SD_MMC2) {
		puts("eMMC\n");
	} else {
		printf("invalid boot source setting\n");
	}

	// Software change for Erratum E-00011
	if (board_revision_num() == REVA)
		puts("VID: Core voltage fixed at 825 mV\n");

	puts("SERDES1 Reference: Clock1 = 100MHz Clock2 = 161.13MHz\n");
	puts("SERDES2 Reference: Clock1 = 100MHz Clock2 = 100MHz\n");
	puts("SERDES3 Reference: Clock1 = 100MHz Clock2 = 100MHz\n");

	return 0;
}

void fdt_fixup_board_model(void *blob)
{
	int board_rev;
	char board_rev_str[64] = {0};

	board_rev = board_revision_num();
	sprintf(board_rev_str, "NXP Layerscape LA1224-RDB Rev%c",
		board_revision_num() + 'A');

	do_fixup_by_path(blob, "/", "model",
			 board_rev_str,
							sizeof(board_rev_str), 1);
}
#else
int checkboard(void)
{
	enum boot_src src = get_boot_src();
	char buf[64];
	u8 sw;
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	int clock;
	static const char *const freq[] = {"100", "125", "156.25",
					   "161.13", "322.26", "", "", "",
					   "", "", "", "", "", "", "",
					   "100 separate SSCG"};
#endif

	cpu_name(buf);
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	printf("Board: %s-QDS, ", buf);
#else
	printf("Board: %s-RDB, ", buf);
#endif

	sw = QIXIS_READ(arch);
	printf("Board version: %c, boot from ", (sw & 0xf) - 1 + 'A');

	if (src == BOOT_SOURCE_SD_MMC) {
		puts("SD\n");
	} else if (src == BOOT_SOURCE_SD_MMC2) {
		puts("eMMC\n");
	} else {
		sw = QIXIS_READ(brdcfg[0]);
		sw = (sw >> QIXIS_XMAP_SHIFT) & QIXIS_XMAP_MASK;
		switch (sw) {
		case 0:
		case 4:
			puts("FlexSPI DEV#0\n");
			break;
		case 1:
			puts("FlexSPI DEV#1\n");
			break;
		case 2:
		case 3:
			puts("FlexSPI EMU\n");
			break;
		default:
			printf("invalid setting, xmap: %d\n", sw);
			break;
		}
	}
#if defined(CONFIG_TARGET_LX2160ARDB)
	printf("FPGA: v%d.%d\n", QIXIS_READ(scver), QIXIS_READ(tagdata));

	puts("SERDES1 Reference: Clock1 = 161.13MHz Clock2 = 161.13MHz\n");
	puts("SERDES2 Reference: Clock1 = 100MHz Clock2 = 100MHz\n");
	puts("SERDES3 Reference: Clock1 = 100MHz Clock2 = 100MHz\n");
#else
	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	puts("SERDES1 Reference : ");
	sw = QIXIS_READ(brdcfg[2]);
	clock = sw >> 4;
	printf("Clock1 = %sMHz ", freq[clock]);
#if defined(CONFIG_TARGET_LX2160AQDS)
	clock = sw & 0x0f;
	printf("Clock2 = %sMHz", freq[clock]);
#endif
	sw = QIXIS_READ(brdcfg[3]);
	puts("\nSERDES2 Reference : ");
	clock = sw >> 4;
	printf("Clock1 = %sMHz ", freq[clock]);
	clock = sw & 0x0f;
	printf("Clock2 = %sMHz\n", freq[clock]);
#if defined(CONFIG_TARGET_LX2160AQDS)
	sw = QIXIS_READ(brdcfg[12]);
	puts("SERDES3 Reference : ");
	clock = sw >> 4;
	printf("Clock1 = %sMHz Clock2 = %sMHz\n", freq[clock], freq[clock]);
#endif
#endif /* CONFIG_TARGET_LX2160ARDB */
	return 0;
}
#endif /* CONFIG_TARGET_LA1224RDB */

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
/*
 * implementation of CONFIG_ESDHC_DETECT_QUIRK Macro.
 */
u8 qixis_esdhc_detect_quirk(void)
{
	/*
	 * SDHC1 Card ID:
	 * Specifies the type of card installed in the SDHC1 adapter slot.
	 * 000= (reserved)
	 * 001= eMMC V4.5 adapter is installed.
	 * 010= SD/MMC 3.3V adapter is installed.
	 * 011= eMMC V4.4 adapter is installed.
	 * 100= eMMC V5.0 adapter is installed.
	 * 101= MMC card/Legacy (3.3V) adapter is installed.
	 * 110= SDCard V2/V3 adapter installed.
	 * 111= no adapter is installed.
	 */
	return ((QIXIS_READ(sdhc1) & QIXIS_SDID_MASK) !=
		 QIXIS_ESDHC_NO_ADAPTER);
}

static void esdhc_adapter_card_ident(void)
{
	u8 card_id, val;

	val = QIXIS_READ(sdhc1);
	card_id = val & QIXIS_SDID_MASK;

	switch (card_id) {
	case QIXIS_ESDHC_ADAPTER_TYPE_SD:
		/* Power cycle to card */
		val &= ~QIXIS_SDHC1_S1V3;
		QIXIS_WRITE(sdhc1, val);
		mdelay(1);
		val |= QIXIS_SDHC1_S1V3;
		QIXIS_WRITE(sdhc1, val);
		/* Route to SDHC1_VS */
		val = QIXIS_READ(brdcfg[11]);
		val |= QIXIS_SDHC1_VS;
		QIXIS_WRITE(brdcfg[11], val);
		break;
	default:
		break;
	}
}

int config_board_mux(void)
{
	u8 reg11, reg5, reg13;
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 sdhc1_base_pmux;
	u32 sdhc2_base_pmux;
	u32 iic5_pmux;

	/* Routes {I2C2_SCL, I2C2_SDA} to SDHC1 as {SDHC1_CD_B, SDHC1_WP}.
	 * Routes {I2C3_SCL, I2C3_SDA} to CAN transceiver as {CAN1_TX,CAN1_RX}.
	 * Routes {I2C4_SCL, I2C4_SDA} to CAN transceiver as {CAN2_TX,CAN2_RX}.
	 * Qixis and remote systems are isolated from the I2C1 bus.
	 * Processor connections are still available.
	 * SPI2 CS2_B controls EN25S64 SPI memory device.
	 * SPI3 CS2_B controls EN25S64 SPI memory device.
	 * EC2 connects to PHY #2 using RGMII protocol.
	 * CLK_OUT connects to FPGA for clock measurement.
	 */

	reg5 = QIXIS_READ(brdcfg[5]);
	reg5 = CFG_MUX_I2C_SDHC(reg5, 0x40);
	QIXIS_WRITE(brdcfg[5], reg5);

	/* Check RCW field sdhc1_base_pmux
	 * esdhc0 : sdhc1_base_pmux = 0
	 * dspi0  : sdhc1_base_pmux = 2
	 */
	sdhc1_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_SDHC1_BASE_PMUX_MASK;
	sdhc1_base_pmux >>= FSL_CHASSIS3_SDHC1_BASE_PMUX_SHIFT;

	if (sdhc1_base_pmux == SDHC1_BASE_PMUX_DSPI) {
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX1_SDHC1_DSPI(reg11, 0x40);
		QIXIS_WRITE(brdcfg[11], reg11);
	} else {
		/* - Routes {SDHC1_CMD, SDHC1_CLK } to SDHC1 adapter slot.
		 *          {SDHC1_DAT3, SDHC1_DAT2} to SDHC1 adapter slot.
		 *          {SDHC1_DAT1, SDHC1_DAT0} to SDHC1 adapter slot.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX1_SDHC1_SDHC(reg11);
		QIXIS_WRITE(brdcfg[11], reg11);
	}

	/* Check RCW field sdhc2_base_pmux
	 * esdhc1 : sdhc2_base_pmux = 0 (default)
	 * dspi1  : sdhc2_base_pmux = 2
	 */
	sdhc2_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR13_REGSR - 1])
		& FSL_CHASSIS3_SDHC2_BASE_PMUX_MASK;
	sdhc2_base_pmux >>= FSL_CHASSIS3_SDHC2_BASE_PMUX_SHIFT;

	if (sdhc2_base_pmux == SDHC2_BASE_PMUX_DSPI) {
		reg13 = QIXIS_READ(brdcfg[13]);
		reg13 = SET_CFG_MUX_SDHC2_DSPI(reg13, 0x01);
		QIXIS_WRITE(brdcfg[13], reg13);
	} else {
		reg13 = QIXIS_READ(brdcfg[13]);
		reg13 = SET_CFG_MUX_SDHC2_DSPI(reg13, 0x00);
		QIXIS_WRITE(brdcfg[13], reg13);
	}

	/* Check RCW field IIC5 to enable dspi2 DT nodei
	 * dspi2: IIC5 = 3
	 */
	iic5_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_IIC5_PMUX_MASK;
	iic5_pmux >>= FSL_CHASSIS3_IIC5_PMUX_SHIFT;

	if (iic5_pmux == IIC5_PMUX_SPI3) {
		/* - Routes {SDHC1_DAT4} to SPI3 devices as {SPI3_M_CS0_B}. */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX2_SDHC1_SPI(reg11, 0x10);
		QIXIS_WRITE(brdcfg[11], reg11);

		/* - Routes {SDHC1_DAT5, SDHC1_DAT6} nowhere.
		 * {SDHC1_DAT7, SDHC1_DS } to {nothing, SPI3_M0_CLK }.
		 * {I2C5_SCL, I2C5_SDA } to {SPI3_M0_MOSI, SPI3_M0_MISO}.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX3_SDHC1_SPI(reg11, 0x01);
		QIXIS_WRITE(brdcfg[11], reg11);
	} else {
		/*
		 * If {SDHC1_DAT4} has been configured to route to SDHC1_VS,
		 * do not change it.
		 * Otherwise route {SDHC1_DAT4} to SDHC1 adapter slot.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		if ((reg11 & 0x30) != 0x30) {
			reg11 = SET_CFG_MUX2_SDHC1_SPI(reg11, 0x00);
			QIXIS_WRITE(brdcfg[11], reg11);
		}

		/* - Routes {SDHC1_DAT5, SDHC1_DAT6} to SDHC1 adapter slot.
		 * {SDHC1_DAT7, SDHC1_DS } to SDHC1 adapter slot.
		 * {I2C5_SCL, I2C5_SDA } to SDHC1 adapter slot.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX3_SDHC1_SPI(reg11, 0x00);
		QIXIS_WRITE(brdcfg[11], reg11);
	}

	return 0;
}

int board_early_init_r(void)
{
	esdhc_adapter_card_ident();
	return 0;
}
#elif defined(CONFIG_TARGET_LX2160ARDB)
int config_board_mux(void)
{
	u8 brdcfg;

	brdcfg = QIXIS_READ(brdcfg[4]);
	/* The BRDCFG4 register controls general board configuration.
	 *|-------------------------------------------|
	 *|Field  | Function                          |
	 *|-------------------------------------------|
	 *|5      | CAN I/O Enable (net CFG_CAN_EN_B):|
	 *|CAN_EN | 0= CAN transceivers are disabled. |
	 *|       | 1= CAN transceivers are enabled.  |
	 *|-------------------------------------------|
	 */
	brdcfg |= BIT_MASK(5);
	QIXIS_WRITE(brdcfg[4], brdcfg);

	return 0;
}
#elif defined(CONFIG_TARGET_LA1224RDB)
static void init_dspi2(void)
{
	/* extended spi mode with all CS signals inactive state is high */
	u32 mcr_val;
	u32 mcr_cfg_val = (DSPI_MCR_MSTR | DSPI_MCR_PCSIS_MASK |
			DSPI_MCR_DTXF | DSPI_MCR_DRXF |
			DSPI_MCR_CRXF | DSPI_MCR_CTXF |
			DSPI_MCR_XSPI | DSPI_MCR_HALT);

	mcr_val = in_le32(SPI_MCR_REG);
	mcr_val |= DSPI_MCR_HALT;
	out_le32(SPI_MCR_REG, mcr_val);
	out_le32(SPI_MCR_REG, mcr_cfg_val);

	u32 ctar_cfg_val = (DSPI_CTAR_TRSZ(0x0F) | DSPI_CTAR_PCSSCK(0x3) |
			DSPI_CTAR_BR(0x7));
	out_le32(SPI_CTAR0_REG, ctar_cfg_val);

	u32 ctarx_cfg_val = DSPI_CTAR_X_TRSZ | DSPI_CTAR_X_DTCP(0x1);

	out_le32(SPI_CTARE0_REG, ctarx_cfg_val);

	u32 src_cfg_val = (DSPI_SR_TCF | DSPI_SR_EOQF | DSPI_SR_TFFF |
			DSPI_SR_CMDTCF | DSPI_SR_SPEF | DSPI_SR_RFOF |
			DSPI_SR_TFIWF | DSPI_SR_RFDF | DSPI_SR_CMDFFF);

	out_le32(SPI_SR_REG, src_cfg_val);

	mcr_val = in_le32(SPI_MCR_REG);
	mcr_val &= ~DSPI_MCR_HALT;
	out_le32(SPI_MCR_REG, mcr_val);
}

int config_board_mux(void)
{
	init_dspi2();
	return 0;
}

#if defined(CONFIG_BOARD_EARLY_INIT_R)
int board_early_init_r(void)
{
	return 0;
}
#endif
#else
/* TBD for CONFIG_TARGET_LA1238RDB */
#if defined(CONFIG_BOARD_EARLY_INIT_R)
int board_early_init_r(void)
{
	return 0;
}
#endif
int config_board_mux(void)
{
	return 0;
}
#endif

#if CONFIG_IS_ENABLED(TARGET_LX2160ARDB)
u8 get_board_rev(void)
{
	u8 board_rev = (QIXIS_READ(arch) & 0xf) - 1 + 'A';

	return board_rev;
}
#endif

unsigned long get_board_sys_clk(void)
{
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x03) {
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	}
	return 100000000;
#else
	return 100000000;
#endif
}

unsigned long get_board_ddr_clk(void)
{
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 100000000;
#else
	return 100000000;
#endif
}

void set_phy_irq_polarity_inv(u32 irq_mask)
{
	u32 __iomem *irq_ccsr = (u32 __iomem *)ISC_BASE;
	u32 irq_cr = in_le32(irq_ccsr + IRQCR_OFFSET / 4);

	/* use irq_mask to set the corresponding bits for inverting polarity */
	out_le32(irq_ccsr + IRQCR_OFFSET / 4, irq_cr | irq_mask);
}

#if defined(CONFIG_TARGET_LA1224RDB)
/*
 * Software Changes for ERRATA E-000014:HS DCS Firmware loading
 */
void gpio1_29_bit_toggle(void)
{
	u8 t_max = TCLK_TOGGLE_MAX_COUNT;
	void __iomem *gpibe_addr = NULL, *gpodr_addr = NULL, *gpdir_addr = NULL;

	gpibe_addr = (u32 __iomem *)GPIO1_INPUT_BUFFER_ENABLE;
	gpdir_addr = (u32 __iomem *)GPIO1_DIR_REG;
	gpodr_addr = (u32 __iomem *)GPIO1_DATA_REG;

	if (board_revision_num() == REVB)
		mdelay(2000);
	// Enable GPIO buffer
	out_le32(gpibe_addr, in_le32(GPIO1_INPUT_BUFFER_ENABLE)
					| GPIO1_ENABLE_INPUT);
	// Set Output direction
	out_le32(gpdir_addr, in_le32(GPIO1_DIR_REG)
				| GPIO1_PIN_29_HIGH);

	if (board_revision_num() == REVA) {
		while (t_max--) {
			// Drive GPIO PIN high
			out_le32(gpodr_addr, in_le32(GPIO1_DATA_REG)
						| GPIO1_PIN_29_HIGH);
			mdelay(1);
			// Drive GPIO PIN low
			out_le32(gpodr_addr, in_le32(GPIO1_DATA_REG)
					& (~GPIO1_PIN_29_HIGH));
			mdelay(1);
			// Drive GPIO PIN  high
			out_le32(gpodr_addr, in_le32(GPIO1_DATA_REG)
						| GPIO1_PIN_29_HIGH);
			mdelay(1);
			debug("%s: GPIO Toggle count (count %d)\n",
			      __func__, t_max);
		}
	} else {
		// RevB JTAG TCK Workaround
		// Drive GPIO PIN high
		out_le32(gpodr_addr, in_le32(GPIO1_DATA_REG)
					| GPIO1_PIN_29_HIGH);
		// Drive GPIO PIN low
		out_le32(gpodr_addr, in_le32(GPIO1_DATA_REG)
					& (~GPIO1_PIN_29_HIGH));
		mdelay(10);
		// Drive GPIO PIN  high
		out_le32(gpodr_addr, in_le32(GPIO1_DATA_REG)
				| GPIO1_PIN_29_HIGH);
	}
	// Set input direction
	out_le32(gpdir_addr, GPIO1_PIN_29_LOW);
}
#endif /* CONFIG_TARGET_LA1224RDB */
#if defined(CONFIG_TARGET_LX2160ARDB) && defined(CONFIG_QSFP_EEPROM) && defined(CONFIG_PHY_CORTINA)
void qsfp_cortina_detect(void)
{
	u8 qsfp_compat_code;

	/* read qsfp+ eeprom & update environment for cs4223 init */
	select_i2c_ch_pca9547(I2C_MUX_CH_SEC, 0);
	select_i2c_ch_pca9547_sec(I2C_MUX_CH_QSFP, 0);
	qsfp_compat_code = get_qsfp_compat0();
	switch (qsfp_compat_code) {
	case QSFP_COMPAT_CR4:
		env_set(CS4223_CONFIG_ENV, CS4223_CONFIG_CR4);
		break;
	case QSFP_COMPAT_XLPPI:
	case QSFP_COMPAT_SR4:
		env_set(CS4223_CONFIG_ENV, CS4223_CONFIG_SR4);
		break;
	default:
		/* do nothing if detection fails or not supported*/
		break;
	}
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);
}

#endif /* CONFIG_QSFP_EEPROM & CONFIG_PHY_CORTINA */

int board_init(void)
{

#if defined(CONFIG_FSL_MC_ENET) && (defined(CONFIG_TARGET_LX2160ARDB) || \
		 defined(CONFIG_TARGET_LA1224RDB) || defined(CONFIG_TARGET_LA1238RDB) )
	u32 __iomem *irq_ccsr = (u32 __iomem *)ISC_BASE;
#endif
#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);

#if defined(CONFIG_FSL_MC_ENET) && (defined(CONFIG_TARGET_LX2160ARDB) || \
		defined(CONFIG_TARGET_LA1224RDB))
	/* invert AQR107 IRQ pins polarity */
	out_le32(irq_ccsr + IRQCR_OFFSET / 4, AQR107_IRQ_MASK);
#elif defined(CONFIG_TARGET_LA1238RDB)
	out_le32(irq_ccsr + IRQCR_OFFSET / 4, AQR113_IRQ_MASK);
#endif
#if defined(CONFIG_QSFP_EEPROM) && defined(CONFIG_PHY_CORTINA)
	qsfp_cortina_detect();
#elif defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_TARGET_LX2160AQDS)
#if defined(CONFIG_QSFP_EEPROM) && defined(CONFIG_PHY_CORTINA) && \
		!defined(CONFIG_TARGET_LX2162AQDS)
	set_phy_irq_polarity_inv(AQR107_IRQ_MASK);
#endif
#endif

#if !defined(CONFIG_SYS_EARLY_PCI_INIT) && defined(CONFIG_DM_ETH)
	pci_init();
#if defined(CONFIG_TARGET_LA1238RDB)
	toggle_la1238_jtag_tck_gpio();
	la1238rdb_gpio_init();
#endif
#endif
	return 0;
}

void detail_board_ddr_info(void)
{
	int i;
	u64 ddr_size = 0;

	puts("\nDDR    ");
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		ddr_size += gd->bd->bi_dram[i].size;
	print_size(ddr_size, "");
	print_ddr_info(0);
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	config_board_mux();
#if defined(CONFIG_TARGET_LA1224RDB)
	// ERRATA E-000014:HS DCS Firmware loading
	gpio1_29_bit_toggle();
#endif
	return 0;
}
#endif

#ifdef CONFIG_VID
u16 soc_get_fuse_vid(int vid_index)
{
	static const u16 vdd[32] = {
		8250,
		7875,
		7750,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		8000,
		8125,
		8250,
		0,      /* reserved */
		8500,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
	};

	return vdd[vid_index];
};
#endif /* CONFIG_VID */

#ifdef CONFIG_FSL_MC_ENET
extern int fdt_fixup_board_phy(void *fdt);

void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/soc/fsl-mc");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl-mc");

	if (offset < 0) {
		printf("%s: fsl-mc node not found in device tree (error %d)\n",
		       __func__, offset);
		return;
	}

	if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0)) {
		fdt_status_okay(fdt, offset);
#ifndef CONFIG_DM_ETH
		fdt_fixup_board_phy(fdt);
#else
#if defined(CONFIG_TARGET_LX2160ARDB)
			fdt_fixup_board_phy_revc(fdt);
#endif /* CONFIG_TARGET_LX2160ARDB */
#endif /* CONFIG_DM_ETH */
	} else {
		fdt_status_fail(fdt, offset);
	}
}

void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}
#endif /* CONFIG_FSL_MC_ENET */

#if CONFIG_IS_ENABLED(TARGET_LX2160ARDB)
int fdt_fixup_add_thermal(void *blob, int mux_node, int channel, int reg)
{
	int err;
	int noff;
	int offset;
	char channel_node_name[50];
	char thermal_node_name[50];
	u32 phandle;

	snprintf(channel_node_name, sizeof(channel_node_name),
		 "i2c@%x", channel);
	debug("channel_node_name = %s\n", channel_node_name);

	snprintf(thermal_node_name, sizeof(thermal_node_name),
		 "temperature-sensor@%x", reg);
	debug("thermal_node_name = %s\n", thermal_node_name);

	err = fdt_increase_size(blob, 200);
	if (err) {
		printf("fdt_increase_size: err=%s\n", fdt_strerror(err));
		return err;
	}

	noff = fdt_subnode_offset(blob, mux_node, (const char *)
				  channel_node_name);
	if (noff < 0) {
		/* channel node not found - create it */
		noff = fdt_add_subnode(blob, mux_node, channel_node_name);
		if (noff < 0) {
			printf("fdt_add_subnode: err=%s\n", fdt_strerror(err));
			return err;
		}
		fdt_setprop_u32 (blob, noff, "#address-cells", 1);
		fdt_setprop_u32 (blob, noff, "#size-cells", 0);
		fdt_setprop_u32 (blob, noff, "reg", channel);
	}

	/* Create thermal node*/
	offset = fdt_add_subnode(blob, noff, thermal_node_name);
	fdt_setprop(blob, offset, "compatible", "nxp,sa56004",
		    strlen("nxp,sa56004") + 1);
	fdt_setprop_u32 (blob, offset, "reg", reg);

	/* fixup phandle*/
	noff = fdt_node_offset_by_compatible(blob, -1, "regulator-fixed");
	if (noff < 0) {
		printf("%s : failed to get phandle\n", __func__);
		return noff;
	}
	phandle = fdt_get_phandle(blob, noff);
	fdt_setprop_u32 (blob, offset, "vcc-supply", phandle);

	return 0;
}

void fdt_fixup_delete_thermal(void *blob, int mux_node, int channel, int reg)
{
	int node;
	int value;
	int err;
	int subnode;

	fdt_for_each_subnode(subnode, blob, mux_node) {
		value = fdtdec_get_uint(blob, subnode, "reg", -1);
		if (value == channel) {
			/* delete thermal node */
			fdt_for_each_subnode(node, blob, subnode) {
				value = fdtdec_get_uint(blob, node, "reg", -1);
				err = fdt_node_check_compatible(blob, node,
								"nxp,sa56004");
				if (!err && value == reg) {
					fdt_del_node(blob, node);
					break;
				}
			}
		}
	}
}

void fdt_fixup_i2c_thermal_node(void *blob)
{
	int i2coffset;
	int mux_node;
	int reg;
	int err;

	i2coffset = fdt_node_offset_by_compat_reg(blob, "fsl,vf610-i2c",
						  0x2000000);
	if (i2coffset != -FDT_ERR_NOTFOUND) {
		fdt_for_each_subnode(mux_node, blob, i2coffset) {
			reg = fdtdec_get_uint(blob, mux_node, "reg", -1);
			err = fdt_node_check_compatible(blob, mux_node,
							"nxp,pca9547");
			if (!err && reg == 0x77) {
				fdt_fixup_delete_thermal(blob, mux_node,
							 0x3, 0x4d);
				err = fdt_fixup_add_thermal(blob, mux_node,
							    0x3, 0x48);
				if (err)
					printf("%s: Add thermal node failed\n",
					       __func__);
			}
		}
	} else {
		printf("%s: i2c node not found\n", __func__);
	}
}
#endif /* TARGET_LX2160ARDB */

#define LX2XX_MAX_CORES 16
#define VALS_PER_MAP 3
void disable_cpu_thermal_maps(void *blob)
{
	static const char* const cool_maps_path[] = {
		"/thermal-zones/cluster6-7/cooling-maps/map0"
	};
	int nodeoff = 0, cnt = 0, i= 0, ret = 0, j = 0, index = 0;
    u32 cooling_dev[LX2XX_MAX_CORES * VALS_PER_MAP] = {0};
    int num_cores = cpu_numcores();
	int disabled_maps = (LX2XX_MAX_CORES - num_cores);

	/* For 16 Cores no fixup required. */
    if (num_cores == LX2XX_MAX_CORES)
		return;

	for (i = 0; i < ARRAY_SIZE(cool_maps_path); i++) {
		nodeoff = fdt_path_offset(blob,cool_maps_path[i]);
		if (nodeoff < 0)
			continue; /*  Not found, skip it */

		cnt = fdtdec_get_int_array_count(blob, nodeoff,
				"cooling-device", cooling_dev,
				(LX2XX_MAX_CORES * VALS_PER_MAP));
		if (cnt < 0)
			continue;

        if (cnt != (LX2XX_MAX_CORES * VALS_PER_MAP))
            printf("Warning: %s, cooling-device count %d\n",
                    cool_maps_path[i], cnt);

        if (num_cores != 8)	{
            for (j = 0; j < cnt; j++) {
                cooling_dev[j] = cpu_to_fdt32(cooling_dev[j]);
            }
        } else {
            /* Maps are already as per no. of cores */
            if (cnt == (num_cores * VALS_PER_MAP))
                return;

            /* For 8 Core personality skip alternate cooling maps */
            for (j = 0, i = 0; j < num_cores * 3; j++) {
                    index = ((i * 2) + (j - i));
                    cooling_dev[j] = cpu_to_fdt32(cooling_dev[index]);
                    ((j + 1) % 3) ? i : (i += 3);
            }
        }

		ret = fdt_setprop(blob, nodeoff, "cooling-device", &cooling_dev,
					sizeof(u32) * (VALS_PER_MAP *
					( LX2XX_MAX_CORES - disabled_maps)));
		if (ret < 0) {
			printf("Warning: %s, cooling-device setprop failed %d\n",
					cool_maps_path[i], ret);
		}
	}
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int i;
	u16 mc_memory_bank = 0;

	u64 *base;
	u64 *size;
	u64 mc_memory_base = 0;
	u64 mc_memory_size = 0;
	u16 total_memory_banks;
	int err;

	err = fdt_increase_size(blob, 512);
	if (err) {
		printf("%s fdt_increase_size: err=%s\n", __func__,
		       fdt_strerror(err));
		return err;
	}

	ft_cpu_setup(blob, bd);

	fdt_fixup_mc_ddr(&mc_memory_base, &mc_memory_size);

	if (mc_memory_base != 0)
		mc_memory_bank++;

	total_memory_banks = CONFIG_NR_DRAM_BANKS + mc_memory_bank;

	base = calloc(total_memory_banks, sizeof(u64));
	size = calloc(total_memory_banks, sizeof(u64));

	/* fixup DT for the three GPP DDR banks */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
	else if (gd->arch.resv_ram >= base[2] &&
		 gd->arch.resv_ram < base[2] + size[2])
		size[2] = gd->arch.resv_ram - base[2];
#endif

	if (mc_memory_base != 0) {
		for (i = 0; i <= total_memory_banks; i++) {
			if (base[i] == 0 && size[i] == 0) {
				base[i] = mc_memory_base;
				size[i] = mc_memory_size;
				break;
			}
		}
	}

	fdt_fixup_memory_banks(blob, base, size, total_memory_banks);

#ifdef CONFIG_USB_HOST
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_FSL_MC_ENET
	fdt_fsl_mc_fixup_iommu_map_entry(blob);
	fdt_fixup_board_enet(blob);
#endif
	fdt_fixup_icid(blob);

#if defined(CONFIG_TARGET_LA1238RDB) || defined(CONFIG_TARGET_LA1224RDB)
	fdt_fixup_board_model(blob);
#endif
#if CONFIG_IS_ENABLED(TARGET_LX2160ARDB)
	if (get_board_rev() >= 'C')
		fdt_fixup_i2c_thermal_node(blob);
#endif
	disable_cpu_thermal_maps(blob);
	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */
#if !defined(CONFIG_TARGET_LA1238RDB)
void qixis_dump_switch(void)
{
	int i, nr_of_cfgsw;

	QIXIS_WRITE(cms[0], 0x00);
	nr_of_cfgsw = QIXIS_READ(cms[1]);

	puts("DIP switch settings dump:\n");
	for (i = 1; i <= nr_of_cfgsw; i++) {
		QIXIS_WRITE(cms[0], i);
		printf("SW%d = (0x%02x)\n", i, QIXIS_READ(cms[1]));
	}
}
#endif

#if defined(CONFIG_TARGET_LA1238RDB)
int get_pcal_bus(void)
{
	u32 rev = get_board_version();

	switch (rev) {
	case REVA:
		return PCAL_BUS_NO;
	case REVB:
		return PCAL_BUS_NO_REVB;
	default:
		return -1;
	}
}

static int switch_boot_source(int src_id, int param)
{
	int ret;
	struct udevice *dev;
	u8 data;
	int pcal_bus;

	if ((get_board_version() == REVA) && (src_id == BOOT_FROM_SD)) {
		printf("SD boot is not defined for REV A\n");
		return CMD_RET_FAILURE;
	}

	pcal_bus = get_pcal_bus();
	if (pcal_bus == -1) {
		printf("Unable to get pcal bus\n");
		return -ENXIO;
	}

	ret = i2c_get_chip_for_busnum(pcal_bus, PCAL_CPU_ADDR, 1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d, addr :%d, ret: %d\n",
		       __func__, pcal_bus, PCAL_CPU_ADDR, ret);
		return -ENXIO;
	}

	/* Make all rcw src pin output, don't touch svr pins */
	ret = dm_i2c_read(dev, PCAL_CONFIG, &data, 1);
	if (ret) {
		printf("i2c read error addr: %u reg: %u ret: %d\n",
		       PCAL_CPU_ADDR, PCAL_CONFIG, ret);
		return ret;
	}
	/* Clear bit 4 to 7 (RCW_SRC0 to RCW_SRC3) to make then input */
	data &= 0x0f;
	ret = dm_i2c_write(dev, PCAL_CONFIG, &data, 1);
	if (ret) {
		printf("i2c write error addr: %u reg: %u data: %u, ret: %d\n",
		       PCAL_CPU_ADDR, PCAL_CONFIG, data, ret);
		return ret;
	}

	ret = dm_i2c_read(dev, PCAL_OUTPUT_PORT, &data, 1);
	if (ret) {
		printf("i2c read error addr: %u reg: %u ret: %d\n",
		       PCAL_CPU_ADDR, PCAL_OUTPUT_PORT, ret);
		return ret;
	}
	data &= 0x0f;
	switch (src_id) {
	case BOOT_FROM_XSPI: /* RCW_SRC[3:0] = 1111 */
		data |= 0xf0;
		break;
	case BOOT_FROM_SD: /* RCW_SRC[3:0] = 1000 */
		data |= 0x80;
		break;
	case BOOT_FROM_EMMC: /* RCW_SRC[3:0] = 1001 */
		data |= 0x90;
		break;
	default:
		printf("CPU boot source error: %d\n", src_id);
		return -ENXIO;
	}

	ret = dm_i2c_write(dev, PCAL_OUTPUT_PORT, &data, 1);
	if (ret) {
		printf("i2c write error addr: %u reg: %u data: %u, ret: %d\n",
		       PCAL_CPU_ADDR, PCAL_OUTPUT_PORT, data, ret);
		return ret;
	}

	return run_command("reset", 0);
}

static int switch_boot_source_modem(int src_id)
{
	int ret;
	struct udevice *dev;
	u8 data;
	int pcal_bus = get_pcal_bus();

	if (pcal_bus == -1) {
		printf("Unable to get pcal bus\n");
		return -ENXIO;
	}

	ret = i2c_get_chip_for_busnum(pcal_bus, PCAL_MODEM_ADDR, 1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d, addr :%d, ret: %d\n",
		       __func__, pcal_bus, PCAL_MODEM_ADDR, ret);
		return -ENXIO;
	}

	/* Make modem boot source control pins output, Don't touch other pins */
	ret = dm_i2c_read(dev, PCAL_CONFIG, &data, 1);
	if (ret) {
		printf("i2c read error addr: %u reg: %u ret: %d\n",
		       PCAL_MODEM_ADDR, PCAL_CONFIG, ret);
		return ret;
	}
	data = data & 0xf9;
	ret = dm_i2c_write(dev, PCAL_CONFIG, &data, 1);
	if (ret) {
		printf("i2c write error addr: %u reg: %u data: %u, ret: %d\n",
		       PCAL_CPU_ADDR, PCAL_CONFIG, data, ret);
		return ret;
	}

	ret = dm_i2c_read(dev, PCAL_OUTPUT_PORT, &data, 1);
	if (ret) {
		printf("i2c read error addr: %u reg: %u ret: %d\n",
		       PCAL_MODEM_ADDR, PCAL_OUTPUT_PORT, ret);
		return ret;
	}
	data &= 0xf9;
	switch (src_id) {
	case BOOT_FROM_XSPI_MODEM:
		data |= 0x02; /* SW_CFG_BOOT_SRC[1:0] = 01 */
		break;
	case BOOT_FROM_PCIE_MODEM:
		data |= 0x06; /* SW_CFG_BOOT_SRC[1:0] = 11 */
		break;
	case BOOT_FROM_PEB_MODEM:
		data |= 0; /* SW_CFG_BOOT_SRC[1:0] = 00 */
		break;
	default:
		printf("Modem boot source error: %d\n", src_id);
		return -ENXIO;
	}

	ret = dm_i2c_write(dev, PCAL_OUTPUT_PORT, &data, 1);
	if (ret) {
		printf("i2c write error addr: %u reg: %u data: %u, ret: %d\n",
		       PCAL_CPU_ADDR, PCAL_OUTPUT_PORT, data, ret);
		return ret;
	}

	return run_command("reset", 0);
}

static int select_boot_source_modem(struct cmd_tbl *cmdtp, int flag, int argc,
				    char *const argv[])
{
	if (argc <= 1)
		return CMD_RET_USAGE;
	else if (strcmp(argv[1], "xspi") == 0)
		switch_boot_source_modem(BOOT_FROM_XSPI_MODEM);
	else if (strcmp(argv[1], "pcie") == 0)
		switch_boot_source_modem(BOOT_FROM_PCIE_MODEM);
	else if (strcmp(argv[1], "peb") == 0)
		switch_boot_source_modem(BOOT_FROM_PEB_MODEM);
	else
		return CMD_RET_USAGE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(boot_source_modem, 2, 0, select_boot_source_modem,
	   "Modem boot source Selection Control",
	   "boot_source_modem xspi : reset to FlexSPI\n"
	   "boot_source_modem pcie : reset to pcie\n"
	   "boot_source_modem peb : reset to preloaded peb\n"
);
#endif

#if defined(CONFIG_TARGET_LA1224RDB)
static int switch_boot_source(int src_id, int param)
{
	int ret;
	struct udevice *dev;
	u8 data;
	u32 conf_reg;
	u8 cfg_data, out_data;

	switch (board_revision_num() + 'A') {
	case 'C':
		break;
	default:
		printf("Boot source switching is not supported on this board\n");
		return -1;
	};

	ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR_PRI_REVC, 1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus 0 addr :%d, ret: %d\n",
		       __func__, I2C_IO_EXP_ADDR_PRI_REVC, ret);
		return -ENXIO;
	}

	ret = dm_i2c_read(dev, IO_EXAPNDER_P0_CONF_REG_REVC, &cfg_data, 1);
	if (ret) {
		printf("i2c read error addr: %u reg: %u ret: %d\n",
		       I2C_IO_EXP_ADDR_PRI_REVC, IO_EXAPNDER_P0_CONF_REG_REVC,
			ret);

	return ret;
	}
	cfg_data &= 0xe0;

	ret = dm_i2c_read(dev, IO_EXAPNDER_P0_OUTPUT_REG_REVC, &out_data, 1);
	if (ret) {
		printf("i2c read error addr: %u reg: %u ret: %d\n",
		       I2C_IO_EXP_ADDR_PRI_REVC, IO_EXAPNDER_P0_OUTPUT_REG_REVC,
				ret);
		return ret;
	}
	out_data &= 0xe0;

	conf_reg = IO_EXAPNDER_P0_CONF_REG_REVC;
	switch (src_id) {
	case BOOT_FROM_XSPI: /* RCW_SRC[3:0] = 1111 */
		data = cfg_data;
		ret = dm_i2c_write(dev, conf_reg, &data, 1);
		if (param)
			data = (out_data | 0x1f);
		else
			data = (out_data | 0x0f);
		break;
	case BOOT_FROM_SD: /* RCW_SRC[3:0] = 1000 */
		data = (cfg_data | 0x10);
		ret = dm_i2c_write(dev, conf_reg, &data, 1);
		data = (out_data | 0x11);
		break;
	case BOOT_FROM_EMMC: /* RCW_SRC[3:0] = 1001 */
		data = (cfg_data | 0x10);
		ret = dm_i2c_write(dev, conf_reg, &data, 1);
		data = (out_data | 0x19);
		break;
	default:
		printf("CPU boot source error: %d\n", src_id);
		return -ENXIO;
	}

	ret = dm_i2c_write(dev, IO_EXAPNDER_P0_OUTPUT_REG_REVC, &data, 1);
	if (ret) {
		printf("i2c write error addr: %u reg: %u data: %u, ret: %d\n",
		       I2C_IO_EXP_ADDR_PRI_REVC,
					IO_EXAPNDER_P0_OUTPUT_REG_REVC, data, ret);
		return ret;
	}

	return run_command("reset", 0);
}
#endif
#if defined(CONFIG_TARGET_LA1238RDB) || defined(CONFIG_TARGET_LA1224RDB)
static int select_boot_source(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	if (argc <= 1) {
		return CMD_RET_USAGE;
	} else if (strcmp(argv[1], "xspi") == 0) {
		if (argc == 3 && (strcmp(argv[2], "altbank") == 0))
			switch_boot_source(BOOT_FROM_XSPI, 1);
		else
			switch_boot_source(BOOT_FROM_XSPI, 0);
	} else if (strcmp(argv[1], "sd") == 0) {
		switch_boot_source(BOOT_FROM_SD, 0);
	} else if (strcmp(argv[1], "emmc") == 0) {
		switch_boot_source(BOOT_FROM_EMMC, 0);
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(boot_source, 3, 0, select_boot_source,
	   "Boot source Selection Control",
		"boot_source xspi : reset to FlexSPI\n"
		"boot_source sd : reset to sd\n"
		"boot_source emmc : reset to emmc\n"
);
#endif
