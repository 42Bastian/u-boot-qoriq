// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Marvell International Ltd.
 * https://spdx.org/licenses
 */

/*
 * Copyright (C) 2018-2020 MicroSys Electronics GmbH
 *
 */


#include <config.h>
#include <common.h>
#include <command.h>
#include <vsprintf.h>
#include <errno.h>
#include <dm.h>
#include <flash.h>
#include <image.h>
#include <net.h>
#include <spi_flash.h>
#include <spi.h>
#include <nand.h>
#include <usb.h>
#include <fs.h>
#include <mmc.h>
#ifdef CONFIG_BLK
#include <blk.h>
#endif
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>

#ifndef CONFIG_SYS_MMC_ENV_DEV
#define CONFIG_SYS_MMC_ENV_DEV	0
#endif

struct update_dev {
	char name[8];
	size_t (*read)(const char *file_name);
	int (*write)(size_t image_size);
	int (*active)(void);
};

static ulong get_load_addr(void)
{
	const char *addr_str;
	unsigned long addr;

	addr_str = env_get("loadaddr");
	if (addr_str)
		addr = simple_strtoul(addr_str, NULL, 16);
	else
		addr = CONFIG_SYS_LOAD_ADDR;

	return addr;
}

/********************************************************************
 *     eMMC services
 ********************************************************************/
#if CONFIG_IS_ENABLED(DM_MMC) && CONFIG_IS_ENABLED(MMC_WRITE)
static int mmc_burn_image(size_t image_size)
{
	struct mmc	*mmc;
	lbaint_t	start_lba;
	lbaint_t	blk_count;
	ulong		blk_written;
	int		err;
	const u8	mmc_dev_num = CONFIG_SYS_MMC_ENV_DEV;
#ifdef CONFIG_BLK
	struct blk_desc *blk_desc;
#endif
	mmc = find_mmc_device(mmc_dev_num);
	if (!mmc) {
		printf("No SD/MMC/eMMC card found\n");
		return -ENOMEDIUM;
	}

	err = mmc_init(mmc);
	if (err) {
		printf("%s(%d) init failed\n", IS_SD(mmc) ? "SD" : "MMC",
		       mmc_dev_num);
		return err;
	}


	/* SD reserves LBA-0 for MBR and boots from LBA-1,
	 * MMC/eMMC boots from LBA-0
	 */
	start_lba = IS_SD(mmc) ? 1 : 0;
#ifdef CONFIG_BLK
	blk_count = image_size / mmc->write_bl_len;
	if (image_size % mmc->write_bl_len)
		blk_count += 1;

	blk_desc = mmc_get_blk_desc(mmc);
	if (!blk_desc) {
		printf("Error - failed to obtain block descriptor\n");
		return -ENODEV;
	}
	blk_written = blk_dwrite(blk_desc, start_lba, blk_count,
				 (void *)get_load_addr());
#else
	blk_count = image_size / mmc->block_dev.blksz;
	if (image_size % mmc->block_dev.blksz)
		blk_count += 1;

	blk_written = mmc->block_dev.block_write(mmc_dev_num,
						 start_lba, blk_count,
						 (void *)get_load_addr());
#endif /* CONFIG_BLK */
	if (blk_written != blk_count) {
		printf("Error - written %#lx blocks\n", blk_written);
		return -ENOSPC;
	}
	printf("Done!\n");


	return 0;
}

static size_t mmc_read_file(const char *file_name)
{
	loff_t		act_read = 0;
	int		rc;
	struct mmc	*mmc;
	const u8	mmc_dev_num = CONFIG_SYS_MMC_ENV_DEV;

	mmc = find_mmc_device(mmc_dev_num);
	if (!mmc) {
		printf("No SD/MMC/eMMC card found\n");
		return 0;
	}

	if (mmc_init(mmc)) {
		printf("%s(%d) init failed\n", IS_SD(mmc) ? "SD" : "MMC",
		       mmc_dev_num);
		return 0;
	}

	/* Load from data partition (0) */
	if (fs_set_blk_dev("mmc", "0", FS_TYPE_EXT)) {
		printf("Error: MMC 0 not found\n");
		return 0;
	}

	/* Perfrom file read */
	rc = fs_read(file_name, get_load_addr(), 0, 0, &act_read);
	if (rc)
		return 0;

	return act_read;
}

static int is_mmc_active(void)
{
	return 1;
}
#else /* CONFIG_DM_MMC */
static int mmc_burn_image(size_t image_size)
{
	return -ENODEV;
}

static size_t mmc_read_file(const char *file_name)
{
	return 0;
}

static int is_mmc_active(void)
{
	return 0;
}
#endif /* CONFIG_DM_MMC */

/********************************************************************
 *     SPI services
 ********************************************************************/
#ifdef CONFIG_SPI_FLASH
static int spi_burn_image(size_t image_size)
{
	int ret;
	struct spi_flash *flash;
	u32 erase_bytes;

	/* Probe the SPI bus to get the flash device */
	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
	if (!flash) {
		printf("Failed to probe SPI Flash\n");
		return -ENOMEDIUM;
	}

#ifdef CONFIG_SPI_FLASH_PROTECTION
	spi_flash_protect(flash, 0);
#endif
        erase_bytes = image_size +
            (flash->erase_size - image_size % flash->erase_size);
        printf("Erasing %d bytes (%d blocks) at offset 0 ...",
            erase_bytes, erase_bytes / flash->erase_size);
        ret = spi_flash_erase(flash, 0, erase_bytes);
        if (ret){
            printf("Error!\n");
        }
        else{
            printf("Done!\n");

            printf("Writing %d bytes from 0x%lx to offset 0 ...",
                (int)image_size, get_load_addr());
            ret = spi_flash_write(flash, 0, image_size, (void *)get_load_addr());
            if (ret)
                printf("Error!\n");
            else
                printf("Done!\n");
        }

#ifdef CONFIG_SPI_FLASH_PROTECTION
	spi_flash_protect(flash, 1);
#endif

	return ret;
}

static int is_spi_active(void)
{
	return 1;
}

#else /* CONFIG_SPI_FLASH */
static int spi_burn_image(size_t image_size)
{
	return -ENODEV;
}

static int is_spi_active(void)
{
	return 0;
}
#endif /* CONFIG_SPI_FLASH */

/********************************************************************
 *     NAND services
 ********************************************************************/
#ifdef CONFIG_CMD_NAND
static int nand_burn_image(size_t image_size)
{
	int ret;
	uint32_t block_size;
	struct mtd_info *mtd;

	mtd = get_nand_dev_by_index(nand_curr_device);
	if (!mtd) {
		puts("\nno devices available\n");
		return -ENOMEDIUM;
	}
	block_size = mtd->erasesize;

	/* Align U-Boot size to currently used blocksize */
	image_size = ((image_size + (block_size - 1)) & (~(block_size - 1)));

	/* Erase the U-BOOT image space */
	printf("Erasing 0x%x - 0x%x:...", 0, (int)image_size);
	ret = nand_erase(mtd, 0, image_size);
	if (ret) {
		printf("Error!\n");
		goto error;
	}
	printf("Done!\n");

	/* Write the image to flash */
	printf("Writing %d bytes from 0x%lx to offset 0 ... ",
	       (int)image_size, get_load_addr());
	ret = nand_write(mtd, 0, &image_size, (void *)get_load_addr());
	if (ret)
		printf("Error!\n");
	else
		printf("Done!\n");

error:
	return ret;
}

static int is_nand_active(void)
{
	return 1;
}

#else /* CONFIG_CMD_NAND */
static int nand_burn_image(size_t image_size)
{
	return -ENODEV;
}

static int is_nand_active(void)
{
	return 0;
}
#endif /* CONFIG_CMD_NAND */

/********************************************************************
 *     USB services
 ********************************************************************/
#if defined(CONFIG_USB_STORAGE) && defined(CONFIG_BLK)
static size_t usb_read_file(const char *file_name)
{
	loff_t act_read = 0;
	struct udevice *dev;
	int rc;

	usb_stop();

	if (usb_init() < 0) {
		printf("Error: usb_init failed\n");
		return 0;
	}

	/* Try to recognize storage devices immediately */
	blk_first_device(IF_TYPE_USB, &dev);
	if (!dev) {
		printf("Error: USB storage device not found\n");
		return 0;
	}

	/* Always load from usb 0 */
	if (fs_set_blk_dev("usb", "0", FS_TYPE_ANY)) {
		printf("Error: USB 0 not found\n");
		return 0;
	}

	/* Perfrom file read */
	rc = fs_read(file_name, get_load_addr(), 0, 0, &act_read);
	if (rc)
		return 0;

	return act_read;
}

static int is_usb_active(void)
{
	return 1;
}

#else /* defined(CONFIG_USB_STORAGE) && defined (CONFIG_BLK) */
static size_t usb_read_file(const char *file_name)
{
	return 0;
}

static int is_usb_active(void)
{
	return 0;
}
#endif /* defined(CONFIG_USB_STORAGE) && defined (CONFIG_BLK) */

/********************************************************************
 *     Network services
 ********************************************************************/
#ifdef CONFIG_CMD_NET
static size_t tftp_read_file(const char *file_name)
{
    int ret;
	/* update global variable load_addr before tftp file from network */
	image_load_addr = get_load_addr();
    ret = net_loop(TFTPGET);

    if(ret < 0){
        printf("Error file not found, please provide file name with absolute TFTP path!\n");
        return 0;
    }
    else{
        return ret;
    }
}

static int is_tftp_active(void)
{
	return 1;
}

#else
static size_t tftp_read_file(const char *file_name)
{
	return 0;
}

static int is_tftp_active(void)
{
	return 0;
}
#endif /* CONFIG_CMD_NET */

enum update_devices {
	UPDATE_DEV_NET = 0,
	UPDATE_DEV_USB,
	UPDATE_DEV_MMC,
	UPDATE_DEV_SPI,
	UPDATE_DEV_NAND,

	UPDATE_MAX_DEV
};

struct update_dev update_devs[UPDATE_MAX_DEV] = {
	{"tftp", tftp_read_file, NULL, is_tftp_active},
	{"usb",  usb_read_file,  NULL, is_usb_active},
	{"mmc",  mmc_read_file,  NULL, is_mmc_active},
	{"spi",  NULL, spi_burn_image,  is_spi_active},
	{"nand", NULL, nand_burn_image, is_nand_active},
};

static int update_write_file(struct update_dev *dst, size_t image_size)
{
	if (!dst->write) {
		printf("Error: Write not supported on device %s\n", dst->name);
		return -ENOTSUPP;
	}

	return dst->write(image_size);
}

static int update_read_file(struct update_dev *src)
{
	size_t image_size;

	if (!src->read) {
		printf("Error: Read not supported on device \"%s\"\n",
		       src->name);
		return 0;
	}

	image_size = src->read(net_boot_file_name);
	if (image_size <= 0) {
		printf("Error: Failed to read file %s from %s\n",
		       net_boot_file_name, src->name);
		return 0;
	}

	return image_size;
}

static int update_is_dev_active(struct update_dev *dev)
{
	if (!dev->active) {
		printf("Device \"%s\" not supported by U-BOOT image\n",
		       dev->name);
		return 0;
	}

	if (!dev->active()) {
		printf("Device \"%s\" is inactive\n", dev->name);
		return 0;
	}

	return 1;
}

struct update_dev *find_update_dev(char *dev_name)
{
	int dev;

	for (dev = 0; dev < UPDATE_MAX_DEV; dev++) {
		if (strcmp(update_devs[dev].name, dev_name) == 0)
			return &update_devs[dev];
	}

	return 0;
}

#define DEFAULT_UPDATE_SRC "tftp"
#define CONFIG_FIRMWARE_IMAGE_DFLT_NAME "qspi_firmware.img"
#define DEFAULT_UPDATE_DST "spi"
#ifndef DEFAULT_UPDATE_DST
#ifdef CONFIG_SPI_BOOT
#define DEFAULT_UPDATE_DST "spi"
#elif defined(CONFIG_NAND_BOOT)
#define DEFAULT_UPDATE_DST "nand"
else
#define DEFAULT_UPDATE_DST "error"
#endif
#endif /* DEFAULT_UPDATE_DST */

int do_update_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct update_dev *src, *dst;
	size_t image_size;
	char src_dev_name[8];
	char dst_dev_name[8];
	char *name;
	int  err;


	if (argc < 2)
		copy_filename(net_boot_file_name,
			      CONFIG_FIRMWARE_IMAGE_DFLT_NAME,
			      sizeof(net_boot_file_name));
	else
		copy_filename(net_boot_file_name, argv[1],
			      sizeof(net_boot_file_name));

	if (argc >= 3) {
		strncpy(dst_dev_name, argv[2], 8);
	} else {
		name = DEFAULT_UPDATE_DST;
		strncpy(dst_dev_name, name, 8);
	}

	if (argc >= 4)
		strncpy(src_dev_name, argv[3], 8);
	else
		strncpy(src_dev_name, DEFAULT_UPDATE_SRC, 8);

	/* Figure out the destination device */
	dst = find_update_dev(dst_dev_name);
	if (!dst) {
		printf("Error: Unknown destination \"%s\"\n", dst_dev_name);
		return -EINVAL;
	}

	if (!update_is_dev_active(dst))
		return -ENODEV;

	/* Figure out the source device */
	src = find_update_dev(src_dev_name);
	if (!src) {
		printf("Error: Unknown source \"%s\"\n", src_dev_name);
		return 1;
	}

	if (!update_is_dev_active(src))
		return -ENODEV;

	printf("Burning U-BOOT image \"%s\" from \"%s\" to \"%s\"\n",
	       net_boot_file_name, src->name, dst->name);

	image_size = update_read_file(src);
	if (!image_size)
		return -EIO;


	err = update_write_file(dst, image_size);
	if (err)
		return err;

	return 0;
}

U_BOOT_CMD(
	update_firmware, 4, 0, do_update_cmd,
	"Burn a complete qspi or nand firmware image to flashes",
	"[file-name] [destination [source]]\n"
	"\t-file-name     The image file name to burn. spi = qspi_firmware.img, nand=nand_firmware.img\n"
	"\t-destination   Flash to burn to [spi, nand]. Default = spi\n"
	"\t-source        The source to load image from [tftp, mmc]. Default = tftp\n"
	"Example Commands:\n"
	"\tupdate_firmware - Burn qspi_firmware.img or nand_firmware.img (depending on active boot device) from tftp to active boot device\n"
	"\tupdate_firmware nand_firmware.img nand - Burn nand_firmware.img from tftp to NAND flash\n"
	"\tupdate_firmware qspi_firmware.img spi mmc - Burn qspi_firmware.img from mmc to spi\n"

);
