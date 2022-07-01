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

#include "1942.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout bg_layout = 
    {
	16, 16,
	berrn_rgn_frac(1, 3),
	3,
	{berrn_rgn_frac(0, 3), berrn_rgn_frac(1, 3), berrn_rgn_frac(2, 3)},
	{gfx_step8(0, 1), gfx_step8(128, 1)},
	{gfx_step16(0, 8)},
	256
    };

    static BerrnGfxLayout fg_layout =
    {
	8, 8,
	berrn_rgn_frac(1, 1),
	2,
	{4, 0},
	{gfx_step4(0, 1), gfx_step4(8, 1)},
	{gfx_step8(0, 16)},
	128,
    };

    static BerrnGfxLayout obj_layout = 
    {
	16, 16,
	berrn_rgn_frac(1, 2),
	4,
	{berrn_rgn_frac(1, 2) + 4, berrn_rgn_frac(1, 2), 4, 0},
	{gfx_step4(0, 1), gfx_step4(8, 1), gfx_step4(256, 1), gfx_step4(264, 1)},
	{gfx_step16(0, 16)},
	512
    };

    berrn1942video::berrn1942video(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(256, 224);
	bitmap->clear();
    }

    berrn1942video::~berrn1942video()
    {

    }

    void berrn1942video::initPalettes()
    {
	auto color_prom = driver.get_rom_region("colorproms");

	for (int i = 0; i < 256; i++)
	{
	    berrnRGBA color;

	    for (int j = 0; j < 3; j++)
	    {
		uint8_t color_val = color_prom.at(i + (j * 256));
		bool bit0 = testbit(color_val, 0);
		bool bit1 = testbit(color_val, 1);
		bool bit2 = testbit(color_val, 2);
		bool bit3 = testbit(color_val, 3);

		int rgb_val = ((bit0 * 0xE) + (bit1 * 0x1F) + (bit2 * 0x43) + (bit3 * 0x8F));
		color.at(j) = rgb_val;
	    }

	    colors.at(i) = color;
	}

	auto tile_prom = driver.get_rom_region("tileprom");

	for (int i = 0; i < 256; i++)
	{
	    uint8_t prom_val = tile_prom.at(i);
	    for (int j = 0; j < 4; j++)
	    {
		bg_palettes[j][i] = (prom_val | (j * 0x10));
	    }
	}

	auto char_prom = driver.get_rom_region("charprom");

	for (int i = 0; i < 256; i++)
	{
	    uint8_t prom_val = char_prom.at(i);
	    fg_palettes[i] = (0x80 | prom_val);
	}

	auto obj_prom = driver.get_rom_region("objprom");

	for (int i = 0; i < 256; i++)
	{
	    uint8_t prom_val = obj_prom.at(i);
	    obj_palettes[i] = (0x40 | prom_val);
	}
    }

    void berrn1942video::init()
    {
	auto fg_rom = driver.get_rom_region("gfx1");
	auto bg_rom = driver.get_rom_region("gfx2");
	auto obj_rom = driver.get_rom_region("gfx3");
	gfxDecodeSet(bg_layout, bg_rom, bg_tiles);
	gfxDecodeSet(fg_layout, fg_rom, fg_tiles);
	gfxDecodeSet(obj_layout, obj_rom, obj_tiles);
	bg_vram.fill(0);
	fg_vram.fill(0);
	obj_ram.fill(0);
	palette_bank = 0;
	initPalettes();
    }

    void berrn1942video::shutdown()
    {
	bg_tiles.clear();
	fg_tiles.clear();
	obj_tiles.clear();
    }

    void berrn1942video::updatePixels()
    {
	updateBG();
	updateSprites();
	updateFG();
	driver.set_screen_bmp(bitmap);
    }

    void berrn1942video::updateBG()
    {
	for (int xpos = 0; xpos < 256; xpos++)
	{
	    for (int ypos = 0; ypos < 224; ypos++)
	    {
		int sy = ((ypos + 16) % 256);
		int sx = ((xpos + scrollx) % 512);

		int row = (sy / 16);
		int col = (sx / 16);

		// uint32_t offs = ((col * 32) + row);
		uint32_t offs = ((col * 16) + row);
		offs = ((offs & 0xF) | ((offs & 0x1F0) << 1));

		uint32_t color_attrib = bg_vram.at(offs + 0x10);
		uint32_t tile_num = bg_vram.at(offs);

		bool is_flipx = testbit(color_attrib, 5);
		bool is_flipy = testbit(color_attrib, 6);
		int color = (color_attrib & 0x1F);

		tile_num |= (testbit(color_attrib, 7) << 8);

		int py = (sy % 16);
		int px = (sx % 16);

		if (is_flipx)
		{
		    px = (15 - px);
		}

		if (is_flipy)
		{
		    py = (15 - py);
		}

		int pixel = ((py * 16) + px);

		auto &bg_pal = bg_palettes.at(palette_bank);
		uint32_t tile_color = bg_tiles.at((tile_num * 256) + pixel);
		uint8_t palette_color = bg_pal.at((color * 8) + tile_color);
		bitmap->setPixel(xpos, ypos, colors.at(palette_color));
	    }
	}
    }

    void berrn1942video::updateFG()
    {
	for (int xpos = 0; xpos < 256; xpos++)
	{
	    for (int ypos = 0; ypos < 224; ypos++)
	    {
		int sy = ((ypos + 16) % 256);
		int sx = xpos;

		int row = (sy / 8);
		int col = (sx / 8);

		uint32_t offs = ((row * 32) + col);

		uint32_t tile_num = fg_vram.at(offs);
		uint32_t color_attrib = fg_vram.at(0x400 + offs);

		tile_num |= (testbit(color_attrib, 7) << 8);
		int color = (color_attrib & 0x3F);

		int py = (sy % 8);
		int px = (sx % 8);

		int pixel = ((py * 8) + px);

		uint32_t tile_color = fg_tiles.at((tile_num * 64) + pixel);

		if (tile_color == 0)
		{
		    continue;
		}

		uint8_t palette_color = fg_palettes.at((color * 4) + tile_color);
		bitmap->setPixel(xpos, ypos, colors.at(palette_color));
	    }
	}
    }

    void berrn1942video::updateSprites()
    {
	for (int y = 16; y <= 240; y++)
	{
	    clip_miny = (y - 16);
	    clip_maxy = (y - 15);

	    uint8_t objdata[4];
	    uint8_t v = (y - 1);

	    for (int h = 496; h >= 128; h -= 16)
	    {
		bool objcnt4 = (testbit(h, 8) != testbit(~h, 7));
		bool objcnt3 = ((testbit(v, 7) && objcnt4) != testbit(~h, 7));

		uint8_t obj_idx = ((h >> 4) & 0x7);
		obj_idx |= objcnt3 ? 0x08 : 0x00;
		obj_idx |= objcnt4 ? 0x10 : 0x00;
		obj_idx <<= 2;

		for (int i = 0; i < 4; i++)
		{
		    objdata[i] = obj_ram.at(obj_idx | i);
		}

		int sprite_num = (objdata[0] & 0x7F);
		sprite_num |= (testbit(objdata[1], 5) << 7);
		sprite_num |= (testbit(objdata[0], 7) << 8);

		int color = (objdata[1] & 0xF);
		int sx = (objdata[3] - (testbit(objdata[1], 4) << 8));
		int sy = objdata[2];
		int dir = 1;

		uint8_t valpha = uint8_t(sy);
		uint8_t v2c = (uint8_t(~v) + 0xFF);
		uint8_t lvbeta = (v2c + valpha);
		uint8_t vbeta = ~lvbeta;

		bool vleq = (vbeta <= ((~valpha) & 0xFF));
		bool vinlen = true;
		uint8_t vlen = ((objdata[1] >> 6) & 0x3);

		switch (vlen)
		{
		    case 0:
		    {
			vinlen = testbit(lvbeta, 7) && testbit(lvbeta, 6) && testbit(lvbeta, 5) && testbit(lvbeta, 4);
		    }
		    break;
		    case 1:
		    {
			vinlen = testbit(lvbeta, 7) && testbit(lvbeta, 6) && testbit(lvbeta, 5);
		    }
		    break;
		    case 2:
		    {
			vinlen = testbit(lvbeta, 7) && testbit(lvbeta, 6);
		    }
		    break;
		    case 3:
		    {
			vinlen = true;
		    }
		    break;
		}

		bool vinzone = !(vleq && vinlen);

		int pixel = ((objdata[1] >> 6) & 0x3);

		if (pixel == 2)
		{
		    pixel = 3;
		}

		if (!vinzone)
		{
		    for (int i = pixel; i >= 0; i--)
		    {
			renderSprite((sprite_num + i), color, sx, ((sy + (16 * i) * dir) - 16));
		    }
		}
	    }
	}
    }

    void berrn1942video::renderSprite(int sprite_num, int color, int xcoord, int ycoord)
    {
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 256; pixel++)
	{
	    int py = (pixel / 16);
	    int px = (pixel % 16);

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    if (!inRange(ypos, clip_miny, clip_maxy))
	    {
		continue;
	    }

	    uint32_t sprite_color = obj_tiles.at((sprite_num * 256) + pixel);

	    if (sprite_color == 15)
	    {
		continue;
	    }

	    uint8_t pal_color = obj_palettes.at((color * 16) + sprite_color);
	    bitmap->setPixel(xpos, ypos, colors.at(pal_color));
	}
    }

    void berrn1942video::writeScroll(bool is_msb, uint8_t data)
    {
	if (is_msb)
	{
	    scrollx = ((scrollx & 0xFF) | (data << 8));
	}
	else
	{
	    scrollx = ((scrollx & 0xFF00) | data);
	}
    }

    uint8_t berrn1942video::readBG(uint16_t addr)
    {
	addr &= 0x3FF;
	return bg_vram.at(addr);
    }

    void berrn1942video::writeBG(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FF;
	bg_vram.at(addr) = data;
    }

    uint8_t berrn1942video::readFG(uint16_t addr)
    {
	addr &= 0x7FF;
	return fg_vram.at(addr);
    }

    void berrn1942video::writeFG(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FF;
	fg_vram.at(addr) = data;
    }

    uint8_t berrn1942video::readOBJ(uint16_t addr)
    {
	addr &= 0x7F;
	return obj_ram.at(addr);
    }

    void berrn1942video::writeOBJ(uint16_t addr, uint8_t data)
    {
	addr &= 0x7F;
	obj_ram.at(addr) = data;
    }

    void berrn1942video::writePaletteBank(uint8_t data)
    {
	palette_bank = (data & 0x3);
    }
};