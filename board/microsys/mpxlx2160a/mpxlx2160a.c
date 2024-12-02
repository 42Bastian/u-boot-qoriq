// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2020 NXP
 * Copyright (C) 2020 MicroSys Electronics GmbH
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
#include <fsl_sec.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <fsl-mc/fsl_mc.h>
#include <env_internal.h>
#include <efi_loader.h>
#include <asm/arch/mmu.h>
#include <hwconfig.h>
#include <asm/arch/clock.h>
#include <asm/arch/config.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <fsl_immap.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <asm/gic-v3.h>

#define GIC_LPI_SIZE                             0x200000

DECLARE_GLOBAL_DATA_PTR;

static struct pl01x_serial_plat serial0 = {
#if CONFIG_CONS_INDEX == 0
	.base = CONFIG_SYS_SERIAL0,
#elif CONFIG_CONS_INDEX == 1
	.base = CONFIG_SYS_SERIAL1,
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

#define GPIO_IBE(N) (GPIO_BASE(N)+0x18)

int get_serial_clock(void);

static void uart_get_clock(void)
{
	serial0.clock = get_serial_clock();
	serial1.clock = get_serial_clock();
}

int board_early_init_f(void)
{
#ifdef CONFIG_SYS_I2C_EARLY_INIT
	i2c_early_init_f();
#endif
	/* get required clock for UART IP */
	uart_get_clock();

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

	off = fdt_node_offset_by_compatible(fdt, -1, "fsl,lx2160a-pcie");
	while (off != -FDT_ERR_NOTFOUND) {
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
		off = fdt_node_offset_by_compatible(fdt, off,
						    "fsl,lx2160a-pcie");
	}

	return 0;
}
#endif

int esdhc_status_fixup(void *blob, const char *compat)
{
	/* Enable both esdhc DT nodes for LX2160ARDB */
	do_fixup_by_compat(blob, compat, "status", "okay",
			   sizeof("okay"), 1);
	return 0;
}

#define SERDES1_ADDR 0x01EA0000
#define SERDES2_ADDR 0x01EB0000
#define SERDES3_ADDR 0x01EC0000
#define PLLFCR0 0x404
#define PLLSCR0 0x504

/* Get the clock from the Serdes Control Register as string */
static char* get_serdes_clk(u32 *port)
{
	char *clk = "0";
	u32 val = *port;
	val >>= 16;
	val &= 0x1F;
	switch (val) {
		case 0x00:
			clk = "100";
			break;
		case 0x01:
			clk = "125";
			break;
		case 0x02:
			clk = "156.25";
			break;
		case 0x03:
			clk = "150";
			break;
		case 0x04:
			clk = "161.1328125";
			break;
		default:
			break;
	}
	return clk;
}


int checkboard(void)
{
	enum boot_src src = get_boot_src();
	char buf[64];

	cpu_name(buf);
	printf("Board: %s, ", buf);

	switch (src) {
		case BOOT_SOURCE_IFC_NOR:
			puts("IFC NOR\n");
			break;
		case BOOT_SOURCE_IFC_NAND:
			puts("IFC NAND\n");
			break;
		case BOOT_SOURCE_QSPI_NOR:
			puts("QSPI NOR\n");
			break;
		case BOOT_SOURCE_QSPI_NAND:
			puts("QSPI NAND\n");
			break;
		case BOOT_SOURCE_XSPI_NOR:
			puts("XSPI NOR\n");
			break;
		case BOOT_SOURCE_XSPI_NAND:
			puts("XSPI NAND\n");
			break;
		case BOOT_SOURCE_SD_MMC:
			puts("SD\n");
			break;
		case BOOT_SOURCE_SD_MMC2:
			puts("eMMC\n");
			break;
		case BOOT_SOURCE_I2C1_EXTENDED:
			puts("I2C\n");
			break;
		default:
			break;
    }
	printf("\nSERDES1 Reference: Clock1(S) = %sMHz Clock2(F) = %sMHz\n",
		get_serdes_clk((u32*)(SERDES1_ADDR+PLLSCR0)),get_serdes_clk((u32*)(SERDES1_ADDR+PLLFCR0)));
	printf("SERDES2 Reference: Clock1(S) = %sMHz Clock2(F) = %sMHz\n",
		get_serdes_clk((u32*)(SERDES2_ADDR+PLLSCR0)),get_serdes_clk((u32*)(SERDES2_ADDR+PLLFCR0)));
	printf("SERDES3 Reference: Clock1(S) = %sMHz Clock2(F) = %sMHz\n",
		get_serdes_clk((u32*)(SERDES3_ADDR+PLLSCR0)),get_serdes_clk((u32*)(SERDES3_ADDR+PLLFCR0)));

	return 0;
}

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
    return 0;
}
#endif

int config_board_mux(void)
{
	return 0;
}

unsigned long get_board_sys_clk(void)
{
	return 100000000;
}

unsigned long get_board_ddr_clk(void)
{
	return 100000000;
}

int board_init(void)
{

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
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

	return 0;
}
#endif

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
		fdt_fixup_board_phy(fdt);
	} else {
		fdt_status_fail(fdt, offset);
	}
}

void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}
#endif


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

#ifdef CONFIG_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_FSL_MC_ENET
	fdt_fsl_mc_fixup_iommu_map_entry(blob);
	fdt_fixup_board_enet(blob);
#endif
	fdt_fixup_icid(blob);



	return 0;
}
#endif
