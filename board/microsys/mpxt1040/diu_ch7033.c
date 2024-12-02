/*
 *  Copyright (C) 2009-2010 Freescale Semiconductor Ltd.
 *
 *  Copyright (C) 2016 MicroSys GmbH
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <stdio_dev.h>
#include <i2c.h>
#include <div64.h>

/* page 1 regs */
#define	I2C_DVI_PAGE_SELECT_REG			0x03
#define	I2C_DVI_POWER_STATE_1_REG		0x04
#define	I2C_DVI_POWER_STATE_3_REG		0x09
#define	I2C_DVI_POWER_STATE_4_REG		0x0a
#define I2C_DVI_INPUT_TIMING_1_REG		0x0b
#define I2C_DVI_INPUT_TIMING_2_REG		0x0c
#define I2C_DVI_INPUT_TIMING_3_REG		0x0d
#define I2C_DVI_INPUT_TIMING_4_REG		0x0e
#define I2C_DVI_INPUT_TIMING_5_REG		0x0f
#define I2C_DVI_INPUT_TIMING_6_REG		0x10
#define I2C_DVI_INPUT_TIMING_7_REG		0x11
#define I2C_DVI_INPUT_TIMING_8_REG		0x12
#define I2C_DVI_INPUT_TIMING_9_REG		0x13
#define I2C_DVI_INPUT_TIMING_10_REG		0x14
#define I2C_DVI_INPUT_TIMING_11_REG		0x15
#define I2C_DVI_INPUT_TIMING_12_REG		0x16
#define I2C_DVI_INPUT_DATA_FORMAT_1_REG		0x17
#define I2C_DVI_INPUT_DATA_FORMAT_2_REG		0x18
#define I2C_DVI_SYNC_DE_POLARITY_REG		0x19
#define	I2C_DVI_GCLK_FREQ_1_REG			0x1a
#define	I2C_DVI_GCLK_FREQ_2_REG			0x1b
#define	I2C_DVI_CRYSTAL_FREQ_1_REG		0x1c
#define	I2C_DVI_CRYSTAL_FREQ_2_REG		0x1d
#define I2C_DVI_I2S_FORMAT_REG			0x1e
#define I2C_DVI_OUTPUT_TIMING_1_REG		0x1f
#define I2C_DVI_OUTPUT_TIMING_2_REG		0x20
#define I2C_DVI_OUTPUT_TIMING_3_REG		0x21
#define I2C_DVI_OUTPUT_TIMING_4_REG		0x22
#define I2C_DVI_OUTPUT_TIMING_5_REG		0x23
#define I2C_DVI_OUTPUT_TIMING_6_REG		0x24
#define I2C_DVI_OUTPUT_TIMING_7_REG		0x25
#define I2C_DVI_OUTPUT_TIMING_8_REG		0x26
#define I2C_DVI_OUTPUT_TIMING_9_REG		0x27
#define I2C_DVI_OUTPUT_TIMING_10_REG		0x28
#define I2C_DVI_OUTPUT_TIMING_11_REG		0x29
#define I2C_DVI_OUTPUT_TIMING_12_REG		0x2a
#define	I2C_DVI_OUTPUT_VIDEO_FORMAT_REG		0x2b
#define	I2C_DVI_HDTV_OUTPUT_FORMAT_REG		0x2c
#define	I2c_DVI_SDRAM_SETTING_REG		0x2d
#define	I2c_DVI_IMAGE_ROTATION_FLIP_REG		0x2e

#define	I2C_DVI_BRIGTHNESS_ADJUST_REG		0x35
#define	I2C_DVI_CONTRAST_ADJUST_REG		0x36
#define	I2C_DVI_HUE_ADJUST_REG			0x37
#define	I2C_DVI_SATURATION_ADJUST_REG		0x38
#define	I2C_DVI_V_H_POS_ADJ_REG			0x39
#define	I2C_DVI_H_POS_ADJ_2_REG			0x3a
#define	I2C_DVI_V_POS_ADJ_2_REG			0x3b

#define I2C_DVI_OUTPUT_TIMING_DVI_1_REG		0x54
#define I2C_DVI_OUTPUT_TIMING_DVI_2_REG		0x55
#define I2C_DVI_OUTPUT_TIMING_DVI_3_REG		0x56
#define I2C_DVI_OUTPUT_TIMING_DVI_4_REG		0x57
#define I2C_DVI_OUTPUT_TIMING_DVI_5_REG		0x58
#define I2C_DVI_OUTPUT_TIMING_DVI_6_REG		0x59

#define	I2C_DVI_TEST_PATTERN_REG		0x7f

/* page 4 */
#define	I2C_DVI_DEVICE_ID_REG			0x50
#define	I2C_DVI_REVISION_ID_REG			0x51
#define	I2C_DVI_RESET_REG			0x52
#define	I2C_DVI_UCLK_CONTROL_REG		0x61

//Chrontel type define:
typedef unsigned long long	ch_uint64;
typedef unsigned int		ch_uint32;
typedef unsigned short		ch_uint16;
typedef unsigned char		ch_uint8;
typedef ch_uint32		ch_bool;

#define ch_true			0
#define ch_false		1

//int i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len);
//int i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len);

//IIC function: read/write CH7033
ch_uint32 I2CRead(ch_uint32 index)
{
	int ret;
	uchar value;

	ret = i2c_read(CONFIG_SYS_I2C_DVI_ADDR, index, 1, &value, 1);
//	if (ret)
//		puts("i2c_read from CH7033: failed \n");

	return value;
}
void I2CWrite(ch_uint32 index, ch_uint32 value)
{
	int ret;
	u8 temp;

	temp = value;

	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, index, 1, &temp, 1);
//	if (ret)
//		puts("i2c_write to CH7033: failed \n");

}

//1280x720 vga timming
static ch_uint32 CH7033_VGA_RegTable[][2] = {
	{ 0x03, 0x04 },
	{ 0x52, 0xC3 },
	{ 0x5A, 0x06 },
	{ 0x5A, 0x04 },
	{ 0x5A, 0x06 },
	{ 0x52, 0xC1 },
	{ 0x52, 0xC3 },
	{ 0x5A, 0x04 },
	{ 0x03, 0x00 },
	{ 0x07, 0xD9 },
	{ 0x08, 0xF1 },
	{ 0x09, 0x10 },
	{ 0x0A, 0xBE },
	{ 0x0B, 0x24 },
	{ 0x0C, 0x00 },
	{ 0x0D, 0xCB },
	{ 0x0E, 0x00 },
	{ 0x0F, 0x30 },
	{ 0x10, 0x03 },
	{ 0x11, 0x1B },
	{ 0x12, 0x00 },
	{ 0x13, 0x83 },
	{ 0x14, 0x00 },
	{ 0x15, 0x68 },
	{ 0x16, 0x04 },
	{ 0x17, 0x01 },
	{ 0x18, 0x00 },
	{ 0x19, 0x0c },
	{ 0x1A, 0xfd },
	{ 0x1B, 0xe8 },
	{ 0x1C, 0x69 },
	{ 0x1D, 0x78 },
	{ 0x1E, 0x00 },
	{ 0x1F, 0x24 },
	{ 0x20, 0x00 },
	{ 0x21, 0xCB },
	{ 0x22, 0x00 },
	{ 0x23, 0x10 },
	{ 0x24, 0x60 },
	{ 0x25, 0x1B },
	{ 0x26, 0x00 },
	{ 0x27, 0x83 },
	{ 0x28, 0x00 },
	{ 0x29, 0x0A },
	{ 0x2A, 0x02 },
	{ 0x2B, 0x09 },
	{ 0x2C, 0x00 },
	{ 0x2D, 0x00 },
	{ 0x2E, 0x3D },
	{ 0x2F, 0x00 },
	{ 0x32, 0xC0 },
	{ 0x36, 0x40 },
	{ 0x38, 0x47 },
	{ 0x3D, 0x86 },
	{ 0x3E, 0x00 },
	{ 0x40, 0x0E },
	{ 0x4B, 0x40 },
	{ 0x4C, 0x40 },
	{ 0x4D, 0x80 },
	{ 0x54, 0x80 },
	{ 0x55, 0x30 },
	{ 0x56, 0x03 },
	{ 0x57, 0x00 },
	{ 0x58, 0x68 },
	{ 0x59, 0x04 },
	{ 0x5A, 0x03 },
	{ 0x5B, 0xCF },
	{ 0x5C, 0x5C },
	{ 0x5D, 0x28 },
	{ 0x5E, 0x4E },
	{ 0x60, 0x00 },
	{ 0x61, 0x00 },
	{ 0x64, 0x2D },
	{ 0x68, 0x44 },
	{ 0x6A, 0x40 },
	{ 0x6B, 0x00 },
	{ 0x6C, 0x10 },
	{ 0x6D, 0x00 },
	{ 0x6E, 0xA0 },
	{ 0x70, 0x98 },
	{ 0x74, 0x30 },
	{ 0x75, 0x80 },
	{ 0x7E, 0x0F },
	{ 0x7F, 0x00 },
	{ 0x03, 0x01 },
	{ 0x08, 0x05 },
	{ 0x09, 0x04 },
	{ 0x0B, 0x65 },
	{ 0x0C, 0x4A },
	{ 0x0D, 0x29 },
	{ 0x0F, 0x9C },
	{ 0x12, 0xD4 },
	{ 0x13, 0xA8 },
	{ 0x14, 0x83 },
	{ 0x15, 0x00 },
	{ 0x16, 0x00 },
	{ 0x1A, 0x6C },
	{ 0x1B, 0x00 },
	{ 0x1C, 0x00 },
	{ 0x1D, 0x00 },
	{ 0x23, 0x63 },
	{ 0x24, 0xB4 },
	{ 0x28, 0x4E },
	{ 0x29, 0x20 },
	{ 0x41, 0x60 },
	{ 0x63, 0x2D },
	{ 0x6B, 0x11 },
	{ 0x6C, 0x02 },
	{ 0x03, 0x03 },
	{ 0x26, 0x00 },
	{ 0x28, 0x08 },
	{ 0x2A, 0x00 },
	{ 0x03, 0x04 },
	{ 0x10, 0x00 },
	{ 0x11, 0xF8 },
	{ 0x12, 0x0C },
	{ 0x13, 0x02 },
	{ 0x14, 0x88 },
	{ 0x15, 0x70 },
	{ 0x20, 0x00 },
	{ 0x21, 0x00 },
	{ 0x22, 0x00 },
	{ 0x23, 0x00 },
	{ 0x24, 0x00 },
	{ 0x25, 0x00 },
	{ 0x26, 0x00 },
	{ 0x54, 0xC4 },
	{ 0x55, 0x5B },
	{ 0x56, 0x4D },
	{ 0x60, 0x01 },
	{ 0x61, 0x62 },
};
#define REGTABLE_VGA_LEN	((sizeof(CH7033_VGA_RegTable))/(2*sizeof(ch_uint32)))

/* Programming of Chrontel CH7033 connector */
int diu_set_dvi_encoder(unsigned int pixclock)
{

	ch_uint32 i;
	ch_uint32 val_t;
	ch_uint32 hinc_reg, hinca_reg, hincb_reg;
	ch_uint32 vinc_reg, vinca_reg, vincb_reg;
	ch_uint32 hdinc_reg, hdinca_reg, hdincb_reg;

	for(i=0; i<REGTABLE_VGA_LEN; ++i)
	{
		I2CWrite(CH7033_VGA_RegTable[i][0], CH7033_VGA_RegTable[i][1]);
	}
#if 1
	//2. Calculate online parameters:
	I2CWrite(0x03, 0x00);
	i = I2CRead(0x25);
	I2CWrite(0x03, 0x04);
	//HINCA:
	val_t = I2CRead(0x2A);
	hinca_reg = (val_t << 3) | (I2CRead(0x2B) & 0x07);
	//HINCB:
	val_t = I2CRead(0x2C);
	hincb_reg = (val_t << 3) | (I2CRead(0x2D) & 0x07);
	//VINCA:
	val_t = I2CRead(0x2E);
	vinca_reg = (val_t << 3) | (I2CRead(0x2F) & 0x07);
	//VINCB:
	val_t = I2CRead(0x30);
	vincb_reg = (val_t << 3) | (I2CRead(0x31) & 0x07);
	//HDINCA:
	val_t = I2CRead(0x32);
	hdinca_reg = (val_t << 3) | (I2CRead(0x33) & 0x07);
	//HDINCB:
	val_t = I2CRead(0x34);
	hdincb_reg = (val_t << 3) | (I2CRead(0x35) & 0x07);
	//do not calculate hdinc if down sample disabled
	if(i & (1 << 6))
	{
		if(hdincb_reg == 0)
		{
			return ch_false;
		}
		//hdinc_reg = (ch_uint32)(((ch_uint64)hdinca_reg) * (1 << 20) / hdincb_reg);
		hdinc_reg = (ch_uint32)lldiv(((ch_uint64)hdinca_reg) * (1 << 20), hdincb_reg);
		I2CWrite(0x3C, (hdinc_reg >> 16) & 0xFF);
		I2CWrite(0x3D, (hdinc_reg >>  8) & 0xFF);
		I2CWrite(0x3E, (hdinc_reg >>  0) & 0xFF);
	}
	if(hincb_reg == 0 || vincb_reg == 0)
	{
		return ch_false;
	}
	if(hinca_reg > hincb_reg)
	{
		return ch_false;
	}
	//hinc_reg = (ch_uint32)((ch_uint64)hinca_reg * (1 << 20) / hincb_reg);
	hinc_reg = (ch_uint32)lldiv((ch_uint64)hinca_reg * (1 << 20), hincb_reg);
	//vinc_reg = (ch_uint32)((ch_uint64)vinca_reg * (1 << 20) / vincb_reg);
	vinc_reg = (ch_uint32)lldiv((ch_uint64)vinca_reg * (1 << 20), vincb_reg);
	I2CWrite(0x36, (hinc_reg >> 16) & 0xFF);
	I2CWrite(0x37, (hinc_reg >>  8) & 0xFF);
	I2CWrite(0x38, (hinc_reg >>  0) & 0xFF);
	I2CWrite(0x39, (vinc_reg >> 16) & 0xFF);
	I2CWrite(0x3A, (vinc_reg >>  8) & 0xFF);
	I2CWrite(0x3B, (vinc_reg >>  0) & 0xFF);
#endif
	//3. Start to running:
	I2CWrite(0x03, 0x00);
	val_t = I2CRead(0x0A);
	I2CWrite(0x0A, val_t | 0x80);
	I2CWrite(0x0A, val_t & 0x7F);
	val_t = I2CRead(0x0A);
	I2CWrite(0x0A, val_t & 0xEF);
	I2CWrite(0x0A, val_t | 0x10);
	I2CWrite(0x0A, val_t & 0xEF);

	printf("CH7033 initialized\n");
	udelay(500);

	return ch_true;
}
