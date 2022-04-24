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
	512,
	3,
	{0, 0x20000, 0x40000},
	{gfx_step8(0, 1), gfx_step8(128, 1)},
	{gfx_step16(0, 8)},
	256
    };

    static BerrnGfxLayout fg_layout =
    {
	8, 8,
	512,
	2,
	{4, 0},
	{gfx_step4(0, 1), gfx_step4(8, 1)},
	{gfx_step8(0, 16)},
	128,
    };

    static BerrnGfxLayout obj_layout = 
    {
	16, 16,
	512,
	4,
	{0x40004, 0x40000, 4, 0},
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

    void berrn1942video::init()
    {
	obj_ram.fill(0);
	fg_ram.fill(0);
	bg_ram.fill(0);

	auto fg_rom = driver.get_rom_region("gfx1");
	auto bg_rom = driver.get_rom_region("gfx2");
	auto obj_rom = driver.get_rom_region("gfx3");

	bg_tiles.resize((512 * 16 * 16), 0);
	fg_tiles.resize((512 * 8 * 8), 0);
	obj_tiles.resize((512 * 16 * 16), 0);

	gfxDecodeSet(bg_layout, bg_rom, bg_tiles);
	gfxDecodeSet(fg_layout, fg_rom, fg_tiles);
	gfxDecodeSet(obj_layout, obj_rom, obj_tiles);
	palette_bank = 0;
	scroll_x = 0;
	init_palette();
    }

    void berrn1942video::shutdown()
    {
	bg_tiles.clear();
	fg_tiles.clear();
	obj_tiles.clear();
	bitmap->clear();
    }

    void berrn1942video::init_palette()
    {
	for (int i = 0; i < 4; i++)
	{
	    bg_pal.at(i).fill(0);
	}

	fg_pal.fill(0);
	obj_pal.fill(0);

	auto char_prom = driver.get_rom_region("charprom");

	for (int i = 0; i < 256; i++)
	{
	    fg_pal.at(i) = ((char_prom.at(i) & 0xF) | 0x80);
	}

	auto tile_prom = driver.get_rom_region("tileprom");

	for (int i = 0; i < 256; i++)
	{
	    for (int j = 0; j < 4; j++)
	    {
		bg_pal[j][i] = ((tile_prom.at(i) & 0xF) | (j * 0x10));
	    }
	}

	auto obj_prom = driver.get_rom_region("spriteprom");

	for (int i = 0; i < 256; i++)
	{
	    obj_pal.at(i) = ((obj_prom.at(i) & 0xF) | 0x40);
	}

	auto color_proms = driver.get_rom_region("palproms");

	for (int i = 0; i < 256; i++)
	{
	    berrnRGBA color = black();
	    for (int j = 0; j < 3; j++)
	    {
		auto &prom_color = color_proms.at(i + (j * 256));
		bool bit0 = testbit(prom_color, 0);
		bool bit1 = testbit(prom_color, 1);
		bool bit2 = testbit(prom_color, 2);
		bool bit3 = testbit(prom_color, 3);

		int color_val = ((bit0 * 0x0E) + (bit1 * 0x1F) + (bit2 * 0x43) + (bit3 * 0x8F));
		color.at(j) = color_val;
	    }

	    colors.at(i) = color;
	}
    }

    void berrn1942video::updatePixels()
    {
	updateBackground();
	updateSprites();
	updateForeground();
	driver.set_screen(bitmap);
    }

    void berrn1942video::updateSprites()
    {
	for (int y = 16; y <= 240; y++)
	{
	    clip_min = (y - 16);
	    clip_max = (y - 15);

	    uint8_t objdata[4];

	    uint8_t value = (y - 1);

	    for (int h = 496; h >= 128; h -= 16)
	    {
		bool objcnt4 = (testbit(h, 8) != testbit(~h, 7));
		bool objcnt3 = (testbit(value, 7) && objcnt4) != testbit(~h, 7);
		uint8_t obj_index = ((h >> 4) & 7);

		obj_index |= objcnt3 ? 0x08 : 0x00;
		obj_index |= objcnt4 ? 0x10 : 0x00;
		obj_index <<= 2;

		for (int i = 0; i < 4; i++)
		{
		    objdata[i] = obj_ram.at(obj_index | i);
		}

		int32_t code = (objdata[0] & 0x7F) + ((objdata[1] & 0x20) << 2) + ((objdata[0] & 0x80) << 1);
		int32_t color = (objdata[1] & 0xF);
		int sx = objdata[3] - 0x10 * (objdata[1] & 0x10);
		int sy = objdata[2];

		int dir = 1;

		uint8_t valpha = uint8_t(sy);
		uint8_t v2c = (uint8_t(~value) + 0xFF);
		uint8_t lvbeta = v2c + valpha;
		uint8_t vbeta = ~lvbeta;
		bool vleq = vbeta <= ((~valpha) & 0xFF);
		bool vinlen = true;
		uint8_t vlen = (objdata[1] >> 6);

		switch ((vlen & 3))
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

		int index = ((objdata[1] & 0xC0) >> 6);

		if (index == 2)
		{
		    index = 3;
		}

		if (!vinzone)
		{
		    for (int i = index; i >= 0; i--)
		    {
			drawSprite((code + i), color, sx, ((sy + (16 * i) * dir) - 16));
		    }
		}
	    }
	}
    }

    void berrn1942video::updateForeground()
    {
	for (int row = 0; row < 32; row++)
	{
	    for (int col = 0; col < 32; col++)
	    {
		uint32_t offset = ((32 * row) + col);

		uint32_t tile_num = fg_ram.at(offset);
		uint32_t color_attrib = fg_ram.at(offset + 0x400);

		tile_num |= (testbit(color_attrib, 7) << 8);

		uint32_t color_num = (color_attrib & 0x3F);

		int xpos = (col * 8);
		int ypos = ((row * 8) - 16);

		drawFGTile(tile_num, color_num, xpos, ypos);
	    }
	}
    }

    void berrn1942video::updateBackground()
    {
	for (int col = 0; col < 32; col++)
	{
	    for (int row = 0; row < 16; row++)
	    {
		int offs = ((32 * col) + row);

		int tile_num = bg_ram.at(offs);
		int color_attrib = bg_ram.at(offs + 0x10);

		bool flipy = testbit(color_attrib, 6);
		bool flipx = testbit(color_attrib, 5);

		uint32_t pal_num = ((color_attrib & 0x1F) | (0x20 * palette_bank));

		tile_num |= (testbit(color_attrib, 7) << 8);

		int xpos = (col * 16);
		int ypos = ((row * 16) - 16);

		drawBGTile(tile_num, pal_num, xpos, ypos, flipx, flipy);
	    }
	}
    }

    void berrn1942video::drawBGTile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord, bool flipx, bool flipy)
    {
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 256; pixel++)
	{
	    int py = (pixel / 16);
	    int px = (pixel % 16);

	    if (flipy)
	    {
		py = (15 - py);
	    }

	    if (flipx)
	    {
		px = (15 - px);
	    }

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    xpos = ((xpos - scroll_x) % 512);

	    if (xpos <= -16)
	    {
		xpos += 512;
	    }

	    int color_val = (pal_num & 0x1F);
	    int pal_bank = ((pal_num >> 5) & 0x3);

	    auto tile_color = bg_pal.at(pal_bank);

	    int color_num = bg_tiles.at((tile_num * 256) + pixel);
	    uint8_t color = tile_color.at((color_val * 8) + color_num);
	    bitmap->setPixel(xpos, ypos, colors.at(color));
	}
    }

    void berrn1942video::drawFGTile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord)
    {
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 64; pixel++)
	{
	    int py = (pixel / 8);
	    int px = (pixel % 8);

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    int color_val = (pal_num & 0x3F);

	    uint8_t color_num = fg_tiles.at((tile_num * 64) + pixel);

	    if (color_num == 0)
	    {
		continue;
	    }

	    uint8_t color = fg_pal.at((color_val * 4) + color_num);
	    bitmap->setPixel(xpos, ypos, colors.at(color));
	}
    }

    void berrn1942video::drawSprite(uint32_t sprite_num, uint32_t pal_num, int xcoord, int ycoord)
    {
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 256; pixel++)
	{
	    int py = (pixel / 16);
	    int px = (pixel % 16);

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    if (!inRange(ypos, clip_min, clip_max))
	    {
		continue;
	    }

	    uint8_t color_num = obj_tiles.at((sprite_num * 256) + pixel);

	    if (color_num == 15)
	    {
		continue;
	    }

	    uint8_t color = obj_pal.at((pal_num * 16) + color_num);
	    bitmap->setPixel(xpos, ypos, colors.at(color));
	}
    }

    uint8_t berrn1942video::readBG(uint16_t addr)
    {
	addr &= 0x3FF;
	return bg_ram.at(addr);
    }

    uint8_t berrn1942video::readFG(uint16_t addr)
    {
	addr &= 0x7FF;
	return fg_ram.at(addr);
    }

    uint8_t berrn1942video::readOBJ(uint16_t addr)
    {
	addr &= 0x7F;
	return obj_ram.at(addr);
    }

    void berrn1942video::writeBG(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FF;
	bg_ram.at(addr) = data;
    }

    void berrn1942video::writeFG(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FF;
	fg_ram.at(addr) = data;
    }

    void berrn1942video::writeOBJ(uint16_t addr, uint8_t data)
    {
	addr &= 0x7F;
	obj_ram.at(addr) = data;
    }

    void berrn1942video::setScroll(bool is_msb, uint8_t data)
    {
	if (!is_msb)
	{
	    scroll_x = ((scroll_x & 0xFF00) | data);
	}
	else
	{
	    scroll_x = ((scroll_x & 0xFF) | (data << 8));
	}
    }

    void berrn1942video::setPaletteBank(uint8_t data)
    {
	palette_bank = (data & 3);
    }
};