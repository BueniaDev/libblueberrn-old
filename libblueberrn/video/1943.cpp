/*
    This file is part of libblueberrn.
    Copyright (C) 2022 BueniaDev.

    libblueberrn is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libblueberrn is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libblueberrn.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "1943.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout bg_layout =
    {
	32, 32,
	berrn_rgn_frac(1, 2),
	4,
	{ berrn_rgn_frac(1, 2) + 4, berrn_rgn_frac(1, 2), 4, 0 },
	{ gfx_step4(0, 1),    gfx_step4(8, 1),    gfx_step4(512, 1),  gfx_step4(520, 1),
	  gfx_step4(1024, 1), gfx_step4(1032, 1), gfx_step4(1536, 1), gfx_step4(1544, 1) },
	{ gfx_step32(0, 16) },
	2048
    };

    berrn1943video::berrn1943video(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(256, 224);
	bitmap->clear();
    }

    berrn1943video::~berrn1943video()
    {

    }

    void berrn1943video::init()
    {
	prior_bmp.fill(0);
	tile_rom = driver.get_rom_region("tilerom");
	auto bg_rom = driver.get_rom_region("gfx2");
	auto bg2_rom = driver.get_rom_region("gfx3");
	gfxDecodeSet(bg_layout, bg2_rom, bg2_tiles);
	gfxDecodeSet(bg_layout, bg_rom, bg_tiles);
	initPalettes();
    }

    void berrn1943video::initPalettes()
    {
	auto fg_prom = driver.get_rom_region("fgproms");
	auto bg_prom = driver.get_rom_region("bgproms");
	auto pal_proms = driver.get_rom_region("colorproms");

	for (int index = 0; index < 0x100; index++)
	{
	    berrnRGBA color;

	    for (int entry = 0; entry < 3; entry++)
	    {
		uint8_t color_entry = pal_proms.at((entry * 0x100) + index);
		bool bit0 = testbit(color_entry, 0);
		bool bit1 = testbit(color_entry, 1);
		bool bit2 = testbit(color_entry, 2);
		bool bit3 = testbit(color_entry, 3);

		int color_val = (bit0 * 0xE) + (bit1 * 0x1F) + (bit2 * 0x43) + (bit3 * 0x8F);

		color.at(entry) = color_val;
	    }

	    colors.at(index) = color;
	}

	for (int index = 0; index < 0x100; index++)
	{
	    int pal_msb = (fg_prom.at(0x100 + index) & 0x3);
	    int pal_lsb = (fg_prom.at(index) & 0xF);
	    uint8_t fg_pal = ((pal_msb << 4) | pal_lsb);

	    is_fg_transparent.at(index) = (fg_pal == 0xF);
	    fg_palettes.at(index) = fg_pal;
	}

	for (int index = 0; index < 0x100; index++)
	{
	    int pal_msb = (bg_prom.at(0x100 + index) & 0x3);
	    int pal_lsb = (bg_prom.at(index) & 0xF);

	    uint8_t bg_pal = ((pal_msb << 4) | pal_lsb);
	    bg_palettes.at(index) = bg_pal;
	}
    }

    void berrn1943video::shutdown()
    {
	bg2_tiles.clear();
	tile_rom.clear();
    }

    void berrn1943video::updatePixels()
    {
	prior_bmp.fill(0);
	bitmap->fillcolor(black());
	if (is_bg2_enabled)
	{
	    updateBG2();
	}

	if (is_bg1_enabled)
	{
	    updateBG1();
	}

	driver.set_screen_bmp(bitmap);
    }

    void berrn1943video::updateBG1()
    {
	for (int xpos = 0; xpos < 256; xpos++)
	{
	    for (int ypos = 0; ypos < 224; ypos++)
	    {
		int sy = (((ypos + 16) + bg_scrolly) % 256);
		int sx = ((xpos + bg_scrollx) % 65536);

		int row = (sy / 32);
		int col = (sx / 32);

		uint32_t offs = ((col * 8) + row);

		uint32_t tile_offs = (offs * 2);
		uint8_t attrib = tile_rom.at(tile_offs + 1);
		uint32_t tile_num = tile_rom.at(tile_offs);
		tile_num |= (testbit(attrib, 0) << 8);

		bool is_flipy = testbit(attrib, 7);
		bool is_flipx = testbit(attrib, 6);
		uint32_t color = ((attrib >> 2) & 0xF);

		int py = (sy % 32);
		int px = (sx % 32);

		if (is_flipy)
		{
		    py = (31 - py);
		}

		if (is_flipx)
		{
		    px = (31 - px);
		}

		int pixel = ((py * 32) + px);

		uint32_t tile_color = bg_tiles.at((tile_num * 1024) + pixel);

		int palette_num = ((color * 16) + tile_color);

		if (is_fg_transparent.at(palette_num))
		{
		    continue;
		}

		uint8_t palette_color = fg_palettes.at(palette_num);
		bitmap->setPixel(xpos, ypos, colors.at(palette_color));
		setPriorPixel(xpos, ypos, 1);
	    }
	}
    }

    void berrn1943video::updateBG2()
    {
	for (int xpos = 0; xpos < 256; xpos++)
	{
	    for (int ypos = 0; ypos < 224; ypos++)
	    {
		int sy = ((ypos + 16) % 256);
		int sx = ((xpos + bg2_scrollx) % 65536);

		int row = (sy / 32);
		int col = (sx / 32);

		uint32_t offs = ((col * 8) + row);

		uint32_t tile_offs = (0x8000 + (offs * 2));
		uint8_t attrib = tile_rom.at(tile_offs + 1);
		uint32_t tile_num = tile_rom.at(tile_offs);

		bool is_flipx = testbit(attrib, 6);
		bool is_flipy = testbit(attrib, 7);
		uint32_t color = ((attrib >> 2) & 0xF);

		tile_num %= 0x80;

		int py = (sy % 32);
		int px = (sx % 32);

		if (is_flipx)
		{
		    px = (31 - px);
		}

		if (is_flipy)
		{
		    py = (31 - py);
		}

		int pixel = ((py * 32) + px);

		uint32_t tile_color = bg2_tiles.at((tile_num * 1024) + pixel);
		int palette_num = ((color * 16) + tile_color);
		uint8_t palette_color = bg_palettes.at(palette_num);
		bitmap->setPixel(xpos, ypos, colors.at(palette_color));
		setPriorPixel(xpos, ypos, 0);
	    }
	}
    }

    void berrn1943video::writeBGScrollX(bool is_msb, uint8_t data)
    {
	if (is_msb)
	{
	    bg_scrollx = ((bg_scrollx & 0xFF) | (data << 8));
	}
	else
	{
	    bg_scrollx = ((bg_scrollx & 0xFF00) | data);
	}
    }

    void berrn1943video::writeBGScrollY(uint8_t data)
    {
	bg_scrolly = data;
    }

    void berrn1943video::writeBG2Scroll(bool is_msb, uint8_t data)
    {
	if (is_msb)
	{
	    bg2_scrollx = ((bg2_scrollx & 0xFF) | (data << 8));
	}
	else
	{
	    bg2_scrollx = ((bg2_scrollx & 0xFF00) | data);
	}
    }

    void berrn1943video::writeD806(uint8_t data)
    {
	is_bg1_enabled = testbit(data, 4);
	is_bg2_enabled = testbit(data, 5);
    }
};