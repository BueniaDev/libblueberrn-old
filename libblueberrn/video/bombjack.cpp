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

#include "bombjack.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout bg_layout = 
    {
	16, 16,
	256,
	3,
	{0, 0x10000, 0x20000},
	{ gfx_step8(0, 1), gfx_step8(64, 1) },
	{ gfx_step8(0, 8), gfx_step8(128, 8) },
	256
    };

    static BerrnGfxLayout fg_layout = 
    {
	8, 8,
	512,
	3,
	{0, 0x8000, 0x10000},
	{ gfx_step8(0, 1) },
	{ gfx_step8(0, 8) },
	64
    };

    static BerrnGfxLayout obj_layout = 
    {
	16, 16,
	128,
	3,
	{0, 0x10000, 0x20000},
	{ gfx_step8(0, 1), gfx_step8(64, 1) },
	{ gfx_step8(0, 8), gfx_step8(128, 8) },
	256
    };

    static BerrnGfxLayout obj_layout2 = 
    {
	32, 32,
	32,
	3,
	{0, 0x10000, 0x20000},
	{ gfx_step8(0, 1), gfx_step8(64, 1), gfx_step8(256, 1), gfx_step8(320, 1) },
	{ gfx_step8(0, 8), gfx_step8(128, 8), gfx_step8(512, 8), gfx_step8(640, 8) },
	1024
    };

    bombjackvideo::bombjackvideo(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(256, 224);
	bitmap->clear();

	palette = new BerrnPaletteXBGR444(128);
    }

    bombjackvideo::~bombjackvideo()
    {

    }

    void bombjackvideo::init()
    {
	vram.fill(0);
	cram.fill(0);
	obj_ram.fill(0);
	pal_ram.fill(0);

	bg_tilemap = driver.get_rom_region("bgtilemap");

	auto bg_rom = driver.get_rom_region("tiles");
	gfxDecodeSet(bg_layout, bg_rom, bg_tiles);

	auto fg_rom = driver.get_rom_region("chars");
	gfxDecodeSet(fg_layout, fg_rom, fg_tiles);

	auto obj_rom = driver.get_rom_region("sprites");

	auto obj2_begin = (obj_rom.begin() + 0x1000);
	auto obj2_end = (obj_rom.end());
	vector<uint8_t> obj_rom2(obj2_begin, obj2_end);

	gfxDecodeSet(obj_layout, obj_rom, obj_tiles);
	gfxDecodeSet(obj_layout2, obj_rom2, obj_tiles2);
    }

    void bombjackvideo::shutdown()
    {
	bg_tilemap.clear();
	bg_tiles.clear();
	fg_tiles.clear();
	obj_tiles.clear();
	obj_tiles2.clear();
	bitmap->clear();
    }

    void bombjackvideo::updatePixels()
    {
	updateBackground();
	updateForeground();
	updateSprites();
	driver.set_screen(bitmap);
    }

    void bombjackvideo::updateBackground()
    {
	for (int row = 0; row < 16; row++)
	{
	    for (int col = 0; col < 16; col++)
	    {
		uint32_t offset = ((16 * row) + col);

		uint32_t bg_offs = (((background_number & 0x7) * 0x200) + offset);
		uint32_t tile_number = testbit(background_number, 4) ? bg_tilemap.at(bg_offs) : 0;
		uint8_t color_attrib = bg_tilemap.at(bg_offs + 0x100);

		int pal_num = (color_attrib & 0xF);

		int xpos = (col * 16);
		int ypos = ((row * 16) - 16);

		bool is_flipy = testbit(color_attrib, 7);
		drawBGTile(tile_number, pal_num, xpos, ypos, is_flipy);
	    }
	}
    }

    void bombjackvideo::updateForeground()
    {
	for (int row = 0; row < 32; row++)
	{
	    for (int col = 0; col < 32; col++)
	    {
		uint32_t offset = ((32 * row) + col);

		uint16_t tile_number = vram.at(offset);
		uint8_t color_attrib = cram.at(offset);

		tile_number |= (testbit(color_attrib, 4) << 8);

		int pal_num = (color_attrib & 0xF);

		int xpos = (col * 8);
		int ypos = ((row * 8) - 16);
		drawFGTile(tile_number, pal_num, xpos, ypos);
	    }
	}
    }

    void bombjackvideo::updateSprites()
    {
	for (int offs = 23; offs >= 0; offs--)
	{
	    size_t obj_offs = (offs * 4);

	    uint8_t num_size = obj_ram.at(obj_offs);
	    uint8_t color_attrib = obj_ram.at(obj_offs + 1);
	    int sy = obj_ram.at(obj_offs + 2);
	    int sx = obj_ram.at(obj_offs + 3);

	    bool is_large_obj = testbit(num_size, 7);

	    if (is_large_obj)
	    {
		sy = (225 - sy);
	    }
	    else
	    {
		sy = (241 - sy);
	    }

	    bool is_flipx = testbit(color_attrib, 6);
	    bool is_flipy = testbit(color_attrib, 7);

	    uint32_t sprite_num = (num_size & 0x7F);
	    int pal_num = (color_attrib & 0xF);

	    int xpos = sx;
	    int ypos = (sy - 16);

	    if (is_large_obj)
	    {
		drawLargeSprite(sprite_num, pal_num, xpos, ypos, is_flipx, is_flipy);
	    }
	    else
	    {
		drawNormalSprite(sprite_num, pal_num, xpos, ypos, is_flipx, is_flipy);
	    }
	}
    }

    void bombjackvideo::drawBGTile(uint16_t tile_num, int pal_num, int xcoord, int ycoord, bool flipy)
    {
	pal_num &= 0xF;
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

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    uint8_t color_num = bg_tiles.at((tile_num * 256) + pixel);
	    uint32_t pal_offs = ((pal_num << 3) | color_num);

	    bitmap->setPixel(xpos, ypos, palette->getColor(pal_offs));
	}
    }

    void bombjackvideo::drawFGTile(uint16_t tile_num, int pal_num, int xcoord, int ycoord)
    {
	pal_num &= 0xF;
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 64; pixel++)
	{
	    int py = (pixel / 8);
	    int px = (pixel % 8);

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    uint8_t color_num = fg_tiles.at((tile_num * 64) + pixel);

	    if (color_num == 0)
	    {
		continue;
	    }

	    uint32_t pal_offs = ((pal_num << 3) | color_num);

	    bitmap->setPixel(xpos, ypos, palette->getColor(pal_offs));
	}
    }

    void bombjackvideo::drawLargeSprite(uint16_t sprite_num, int pal_num, int xcoord, int ycoord, bool flipx, bool flipy)
    {
	pal_num &= 0xF;
	int base_x = xcoord;
	int base_y = ycoord;

	sprite_num &= 0x1F;

	for (int pixel = 0; pixel < 1024; pixel++)
	{
	    int py = (pixel / 32);
	    int px = (pixel % 32);

	    if (flipx)
	    {
		px = (31 - px);
	    }

	    if (flipy)
	    {
		py = (31 - py);
	    }

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    uint8_t color_num = obj_tiles2.at((sprite_num * 1024) + pixel);

	    if (color_num == 0)
	    {
		continue;
	    }

	    uint32_t pal_offs = ((pal_num << 3) | color_num);

	    bitmap->setPixel(xpos, ypos, palette->getColor(pal_offs));
	}
    }

    void bombjackvideo::drawNormalSprite(uint16_t sprite_num, int pal_num, int xcoord, int ycoord, bool flipx, bool flipy)
    {
	pal_num &= 0xF;
	int base_x = xcoord;
	int base_y = ycoord;

	sprite_num &= 0x7F;

	for (int pixel = 0; pixel < 256; pixel++)
	{
	    int py = (pixel / 16);
	    int px = (pixel % 16);

	    if (flipx)
	    {
		px = (15 - px);
	    }

	    if (flipy)
	    {
		py = (15 - py);
	    }

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    uint8_t color_num = obj_tiles.at((sprite_num * 256) + pixel);

	    if (color_num == 0)
	    {
		continue;
	    }

	    uint32_t pal_offs = ((pal_num << 3) | color_num);

	    bitmap->setPixel(xpos, ypos, palette->getColor(pal_offs));
	}
    }

    uint8_t bombjackvideo::readVRAM(uint16_t addr)
    {
	addr &= 0x3FF;
	return vram.at(addr);
    }

    void bombjackvideo::writeVRAM(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FF;
	vram.at(addr) = data;
    }

    uint8_t bombjackvideo::readCRAM(uint16_t addr)
    {
	addr &= 0x3FF;
	return cram.at(addr);
    }

    void bombjackvideo::writeCRAM(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FF;
	cram.at(addr) = data;
    }

    void bombjackvideo::writeORAM(uint16_t addr, uint8_t data)
    {
	addr &= 0x7F;
	obj_ram.at((addr - 0x20)) = data;
    }

    void bombjackvideo::writePalette(uint16_t addr, uint8_t data)
    {
	addr &= 0xFF;
	palette->write8(addr, data);
    }

    void bombjackvideo::writeBackground(uint8_t data)
    {
	background_number = data;
    }
};