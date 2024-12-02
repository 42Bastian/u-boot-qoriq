/* -*-C-*- */
/*
 * Copyright (C) 2018-2019 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include "crx06.h"

#include <common.h>
#include <fdt_support.h>
#include <fm_eth.h>
#include <fsl_mdio.h>

#ifdef CONFIG_FMAN_ENET
#include <fm.h>
#endif

#ifdef CONFIG_FSL_MC_ENET
#include <fsl-mc/ldpaa_wriop.h>
#endif

#include "../crx06/mmd.h"

#define NUM_MDIO_BUSSES 2
#define NUM_TGEC_PHYS   2

typedef struct fdt_phy_handle_s {
    const char *name;
    uint32_t phandle;
    int addr;
} fdt_phy_handle_t;

static fdt_phy_handle_t tgec_phy_handle[NUM_TGEC_PHYS] = {
    [0] = {.name = "ethernet-phy@11",.phandle = 0,.addr = 0x11},
    [1] = {.name = "ethernet-phy@10",.phandle = 0,.addr = 0x10},
};

static int board_fdt_del_phy_subnode(void *fdt, const int off, const char *name)
{
    const int off_phy = fdt_subnode_offset(fdt, off, name);
    if (off_phy >= 0) {
        fdt_del_node(fdt, off_phy);
    }

    return 0;
}

static int board_fdt_del_phy_subnodes(void *fdt, const int off)
{
    int i;
    char name[32];

    for (i = 0; i < PHY_MAX_ADDR; i++) {
        sprintf(name, "ethernet-phy@%x", i);
        board_fdt_del_phy_subnode(fdt, off, name);
    }

    return 0;
}

static int board_fdt_del_tgec_subnodes(void *fdt, const int off,
                                       fdt_phy_handle_t * fdt_phy_handle)
{
    int i;

    for (i = 0; i < NUM_TGEC_PHYS; i++) {
        board_fdt_del_phy_subnode(fdt, off, fdt_phy_handle[i].name);
    }

    return 0;
}

typedef struct fdt_alias_cb_args_s {
    void *fdt;
    const char *alias;
    const char *name;
    void *priv;
    int offset;
} fdt_alias_cb_args_t;

typedef int (*fdt_alias_cb)(fdt_alias_cb_args_t * args);

//static int fdt_enumerate_aliases(void *fdt, const char *const alias,
//        fdt_alias_cb cb, void *cb_args)
//{
//    int prop, offset, i;
//    const char *path;
//    const char *name;
//    fdt_alias_cb_args_t args;
//
//    if (!alias) return -1;
//
//    if (fdt_path_offset(fdt, "/aliases") < 0)
//        return -1;
//
//    const int len = strlen(alias);
//
//    args.fdt = fdt;
//    args.alias = alias;
//    args.priv = cb_args;
//
//    for (prop = 0; ; prop++) {
//        offset = fdt_first_property_offset(fdt,
//            fdt_path_offset(fdt, "/aliases"));
//        for (i = 0; i < prop; i++)
//            offset = fdt_next_property_offset(fdt, offset);
//        if (offset < 0) break;
//        path = fdt_getprop_by_offset(fdt, offset, &name, NULL);
//        if (!strncmp(name, alias, len)) {
//            printf("%s: %s %p\n", name, path, name);
//            if (cb) {
//                args.offset = fdt_path_offset(fdt, path);
//                args.name = name;
//                cb(&args);
//            }
//
//        }
//    }
//
//    return 0;
//}

typedef struct mdio_priv_s {
    struct mii_dev *mdio;
    const char *compat;
    fdt_phy_handle_t phy_handle[PHY_MAX_ADDR];
} mdio_priv_t;

//static int fdt_adjust_mdio(fdt_alias_cb_args_t *args)
//{
//    char path[128];
//
//    if (!args) return -1;
//
//    int i;
//    mdio_priv_t *mdio_priv = (mdio_priv_t *) (args->priv);
//
//    fdt_phy_handle_t *fdt_phy_handle = mdio_priv->fdt_phy_handle;
//
//    board_fdt_del_tgec_subnodes(args->fdt, args->offset, fdt_phy_handle);
//
//    const int len = strlen(args->alias);
//
//    const int mdio_num = args->name[len] - '0';
//
//    printf("fdt_adjust_mdio: %s %d %p\n", args->name, mdio_num, args->name);
//
//    if (mdio_num>=0 && mdio_num<NUM_MDIO_BUSSES) {
//        for (i=0; i<NUM_TGEC_PHYS; i++) {
//            if (/*mdio_priv->mdio[mdio_num]->phymap[fdt_phy_handle[i].addr]*/1) {
//                fdt_get_path(args->fdt, args->offset, &path[0], 128);
//                printf("mdio add to %s\n", path);
//                const int off_phy = fdt_add_subnode(args->fdt, args->offset, fdt_phy_handle[i].name);
//                if (off_phy >= 0) {
//                    fdt_appendprop_u32(args->fdt, off_phy, "reg", fdt_phy_handle[i].addr);
//                    fdt_appendprop_string(args->fdt, off_phy, "compatible", mdio_priv->compat[mdio_num]);
//                    fdt_phy_handle[i].phandle = fdt_create_phandle(args->fdt, off_phy);
//                }
//            }
//        }
//    }
//
//    return 0;
//}

//static int fdt_adjust_tgec(fdt_alias_cb_args_t *args)
//{
//    if (!args) return -1;
//
//    fdt_phy_handle_t *fdt_phy_handle = (fdt_phy_handle_t *) (args->priv);
//
//    const int len = strlen(args->alias);
//
//    const int tgec_num = args->name[len] - '0';
//
//    if (tgec_num>=0 && tgec_num<NUM_TGEC_PHYS) {
//        if (fdt_phy_handle[tgec_num].phandle) {
//            if (fdt_setprop_u32(args->fdt, args->offset, "phy-handle", fdt_phy_handle[tgec_num].phandle) != 0)
//                fdt_appendprop_u32(args->fdt, args->offset, "phy-handle", fdt_phy_handle[tgec_num].phandle);
//            if (fdt_setprop_string(args->fdt, args->offset, "phy-connection-type", "xgmii") != 0)
//                fdt_appendprop_string(args->fdt, args->offset, "phy-connection-type", "xgmii");
//        }
//    }
//
//    return 0;
//}

#define fdt_loop_aliases(__FDT, CODE) {                                                      \
        int __prop;                                                                          \
        int __offset;                                                                        \
        const char *__name;                                                                  \
        const char *__path __maybe_unused;                                                   \
        for (__prop = 0; ; __prop++) {                                                       \
            __offset = fdt_first_property_offset(__FDT, fdt_path_offset(fdt, "/aliases"));   \
            int __i;                                                                         \
            for (__i = 0; __i < __prop; __i++)                                               \
                __offset = fdt_next_property_offset(__FDT, __offset);                        \
            if (__offset < 0) break;                                                         \
            __path = fdt_getprop_by_offset(__FDT, __offset, &__name, NULL); {                \
                CODE;                                                                        \
            }                                                                                \
        }                                                                                    \
}

#define fdt_foreach_alias(__FDT, ALIAS, CODE) {                                            \
        int prop;                                                                          \
        int offset, i;                                                                     \
        const char *name;                                                                  \
        const char *path;                                                                  \
        for (prop = 0; ; prop++) {                                                         \
            offset = fdt_first_property_offset(__FDT, fdt_path_offset(fdt, "/aliases"));   \
            for (i = 0; i < prop; i++)                                                     \
                offset = fdt_next_property_offset(__FDT, offset);                          \
            if (offset < 0) break;                                                         \
            path = fdt_getprop_by_offset(__FDT, offset, &name, NULL); {                    \
              const int len = strlen(ALIAS);                                               \
              if (!strncmp(name, ALIAS, len)) {                                            \
                  CODE;                                                                    \
              }                                                                            \
            }                                                                              \
        }                                                                                  \
}

#ifdef CONFIG_FMAN_ENET
static enum fm_port fman_get_port(struct fm_eth *fm_eth)
{
    if (fm_eth->type == FM_ETH_1G_E)
        return (enum fm_port)fm_eth->num;

    if (fm_eth->type == FM_ETH_10G_E)
        return (enum fm_port)(fm_eth->num + 8);

    return NUM_FM_PORTS;
}
#endif

#ifdef CONFIG_FMAN_ENET
static phy_interface_t fman_get_phy_if(struct fm_eth *fm_eth)
{
    enum fm_port port = fman_get_port(fm_eth);

    if (port == NUM_FM_PORTS)
        return PHY_INTERFACE_MODE_NONE;

    return fm_info_get_enet_if(port);
}
#endif

static __maybe_unused void fsl_dpaa_set_status(void *fdt, const int cell_index,
                                               const char *const status)
{
    int node;
    char path[128];

    snprintf(path, 128, "/soc/fsl,dpaa/ethernet@%x", cell_index);

    node = fdt_path_offset(fdt, path);
    if (node >= 0) {
        if (fdt_setprop_string(fdt, node, "status", status) != 0)
            fdt_appendprop_string(fdt, node, "status", status);
    }
}

static void fsl_dpaa_disable_all(void *fdt)
{
    const int dpaa_offset = fdt_path_offset(fdt, "/soc/fsl,dpaa");
    int node;

    if (dpaa_offset < 0)
        return;

    fdt_for_each_subnode(node, fdt, dpaa_offset) {
        if (fdt_setprop_string(fdt, node, "status", "disabled") != 0)
            fdt_appendprop_string(fdt, node, "status", "disabled");
    }
}

static void fdt_putprop_u32(void *fdt, const int offset, const char *const name,
                            const uint32_t val)
{
    if (fdt_setprop_u32(fdt, offset, name, val) != 0)
        fdt_appendprop_u32(fdt, offset, name, val);
}

static void fdt_putprop_string(void *fdt, const int offset,
                               const char *const name, const char *const val)
{
    if (fdt_setprop_string(fdt, offset, name, val) != 0)
        fdt_appendprop_string(fdt, offset, name, val);
}

int board_crx06_fdt_fixup_phy(void *fdt)
{
    char node_name[32];
    char __maybe_unused path[128];
    int __maybe_unused port, __maybe_unused node;
    phy_interface_t __maybe_unused if_type;
    struct eth_device __maybe_unused *eth;
    struct fm_eth __maybe_unused *fm_eth;
    struct phy_device __maybe_unused *phydev;
    const fdt32_t __maybe_unused *cell_index;

    mdio_priv_t mdio_bus[NUM_MDIO_BUSSES];

#ifdef CONFIG_FSL_MC_ENET
    mdio_bus[0].mdio = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO1_NAME);
    mdio_bus[1].mdio = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
#else
    mdio_bus[0].mdio = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
    mdio_bus[1].mdio = miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME);
#endif

    mdio_bus[0].compat = "ethernet-phy-id002b.09ab";
    mdio_bus[1].compat = "ethernet-phy-ieee802.3-c45";

    //    fdt_enumerate_aliases(fdt, "mdio", fdt_adjust_mdio, (void*) &mdio_priv);

    if (fdt_path_offset(fdt, "/aliases") < 0)
        return -1;

    fdt_foreach_alias(fdt, "mdio", const int mdio_num = name[len] - '0';
                      printf("%s: %s\n", name, path);
                      for (i = 0; i < PHY_MAX_ADDR; i++) {
                      mdio_bus[mdio_num].phy_handle[i].addr = -1;
                      mdio_bus[mdio_num].phy_handle[i].name = NULL;
                      mdio_bus[mdio_num].phy_handle[i].phandle = 0;}

                      const int off = fdt_path_offset(fdt, path);
                      if (mdio_num == 0) board_fdt_del_phy_subnodes(fdt, off);
                      else
                      board_fdt_del_tgec_subnodes(fdt, off,
                                                  &tgec_phy_handle[0]);
                      if (mdio_num >= 0 && mdio_num < NUM_MDIO_BUSSES) {

                      for (i = 0; i < NUM_TGEC_PHYS; i++) {
                      if (mdio_bus[mdio_num].
                          mdio->phymap[tgec_phy_handle[i].addr]) {
                      const int off_phy =
                      fdt_add_subnode(fdt, off, tgec_phy_handle[i].name);
                      if (off_phy >= 0) {
                      fdt_appendprop_u32(fdt, off_phy, "reg",
                                         tgec_phy_handle[i].addr);
                      fdt_appendprop_string(fdt, off_phy, "compatible",
                                            mdio_bus[mdio_num].compat);
                      tgec_phy_handle[i].phandle =
                      fdt_create_phandle(fdt, off_phy);
                      mdio_bus[mdio_num].phy_handle[tgec_phy_handle[i].
                                                    addr].addr =
                      tgec_phy_handle[i].addr;
                      mdio_bus[mdio_num].phy_handle[tgec_phy_handle[i].
                                                    addr].phandle =
                      tgec_phy_handle[i].phandle;}
                      }
                      }

                      for (i = 0; i < PHY_MAX_ADDR; i++) {
                      if (mdio_bus[mdio_num].phy_handle[i].addr < 0
                          && mdio_bus[mdio_num].mdio->phymap[i]) {
                      mdio_bus[mdio_num].phy_handle[i].addr = i;
                      snprintf(node_name, 32, "ethernet-phy@%x", i);
                      //                printf("Adding %s\n", node_name);
                      const int off_phy = fdt_add_subnode(fdt, off, node_name);
                      if (off_phy >= 0) {
                      fdt_appendprop_u32(fdt, off_phy, "reg",
                                         mdio_bus[mdio_num].phy_handle[i].addr);
                      mdio_bus[mdio_num].phy_handle[i].phandle =
                      fdt_create_phandle(fdt, off_phy);}
                      }
                      }
                      }

    )
        //    fdt_enumerate_aliases(fdt, "tgec", fdt_adjust_tgec, (void*) &fdt_phy_handle[0]);

        fdt_foreach_alias(fdt, "tgec", const int tgec_num = name[len] - '0';
                          printf("tgec%d: %s\n", tgec_num, path);
                          const int off = fdt_path_offset(fdt, path);
                          if (tgec_num >= 0 && tgec_num < NUM_TGEC_PHYS) {
                          if (tgec_phy_handle[tgec_num].phandle) {
                          fdt_putprop_u32(fdt, off, "phy-handle",
                                          tgec_phy_handle[tgec_num].phandle);
                          fdt_putprop_string(fdt, off, "phy-connection-type",
                                             "xgmii");}
                          }
        )
            // /soc/fman@1a00000 == alias fman0 fdt_get_alias()

            fsl_dpaa_disable_all(fdt);

#ifdef CONFIG_FMAN_ENET
    const char *fman0_path = fdt_get_alias(fdt, "fman0");
    if (fman0_path) {
        //        const int fman0_offset = fdt_path_offset(fdt, fman0_path);
        port = 0;
        do {
            eth = eth_get_dev_by_index(port++);
            if (eth) {
                fm_eth = eth->priv;
                snprintf(node_name, 32, "ethernet@%05llx",
                         (uint64_t) (fm_eth->mac->base) & 0xfffff);
                snprintf(path, 128, "%s/%s", fman0_path, node_name);
                node = fdt_path_offset(fdt, path);
                if (node >= 0) {
                    phydev = fm_eth->phydev;
                    if (phydev) {
                        printf("eth: %s %s %s PHY@%x index=%d %d\n", eth->name,
                               path, phydev->bus->name, phydev->addr,
                               eth->index, fm_eth->num);
                        int mdio_num = 0;
                        for (mdio_num = 0; mdio_num < NUM_MDIO_BUSSES;
                             mdio_num++) {
                            if (strncmp
                                (mdio_bus[mdio_num].mdio->name,
                                 phydev->bus->name, MDIO_NAME_LEN) == 0)
                                break;
                        }
                        if (mdio_num < NUM_MDIO_BUSSES
                            && mdio_bus[mdio_num].phy_handle[phydev->
                                                             addr].addr >= 0) {
                            if (mdio_bus[mdio_num].
                                phy_handle[phydev->addr].phandle) {
                                fdt_putprop_u32(fdt, node, "phy-handle",
                                                mdio_bus[mdio_num].phy_handle
                                                [phydev->addr].phandle);
                                if_type = fman_get_phy_if(fm_eth);
                                fdt_putprop_string(fdt, node,
                                                   "phy-connection-type",
                                                   phy_string_for_interface
                                                   (if_type));
                                fdt_putprop_string(fdt, node, "status", "okay");
                                cell_index =
                                    fdt_getprop(fdt, node, "cell-index", NULL);
                                fsl_dpaa_set_status(fdt,
                                                    fdt32_to_cpu(*cell_index),
                                                    "okay");
                            }
                        }
                    } else {
                        printf("eth: %s %s index=%d %d\n", eth->name, path,
                               eth->index, fm_eth->num);
                        /*
                         * Fixed link ...
                         * Set 'status' to "okay"
                         */
                        cell_index = fdt_getprop(fdt, node, "cell-index", NULL);
                        fsl_dpaa_set_status(fdt, fdt32_to_cpu(*cell_index),
                                            "okay");
                    }

                    //                    /*
                    //                     * Fix list of aliases:
                    //                     */
                    //
                    //                    fdt_loop_aliases(fdt, {
                    //                        if (strncmp(__name, "ethernet", 8)==0) {
                    //                            const int n = __name[8] - '0';
                    //                            if ((port-1)==n) {
                    //                                fdt_putprop_string(fdt, __offset, __name, path);
                    //                                printf("Found: %s: %s\n", __name, path);
                    //                            }
                    //                        }
                    //                    })

                }
            }
        } while (eth);
    }
#endif

    return 0;
}

#ifdef CONFIG_FSL_MC_ENET
int crx06_init_xfi(const char *const dtsec_mdio_name,
                   const char *const tgec_mdio_name, enum wriop_port port,
                   const int addr)
{
    struct mii_dev *mdio;
    int ret = -1;

    if (dtsec_mdio_name) {
        mdio = miiphy_get_dev_by_name(dtsec_mdio_name);
        ret = mv88x3310_init(mdio, addr, MDIO_CLAUSE_22);
    }

    if (ret < 0 && tgec_mdio_name) {
        mdio = miiphy_get_dev_by_name(tgec_mdio_name);
        ret = mv88x3310_init(mdio, addr, MDIO_CLAUSE_45);
    }

    if (ret == 0) {
        wriop_set_phy_address(port, 0, addr);
        wriop_set_mdio(port, mdio);
    } else
        wriop_disable_dpmac(port);

    return ret;
}
#else
int crx06_init_xfi(const char *const dtsec_mdio_name,
                   const char *const tgec_mdio_name, enum fm_port port,
                   const int addr)
{
    struct mii_dev *mdio;
    int ret = -1;

    if (dtsec_mdio_name) {
        mdio = miiphy_get_dev_by_name(dtsec_mdio_name);
        ret = mv88x3310_init(mdio, addr, MDIO_CLAUSE_22);
    }

    if (ret < 0 && tgec_mdio_name) {
        mdio = miiphy_get_dev_by_name(tgec_mdio_name);
        ret = mv88x3310_init(mdio, addr, MDIO_CLAUSE_45);
    }

    if (ret == 0) {
        fm_info_set_phy_address(port, addr);
        fm_info_set_mdio(port, mdio);
    } else
        fm_disable_port(port);

    return ret;
}
#endif

/*!@} */

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
