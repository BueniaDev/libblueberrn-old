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

#include "galaxian.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout char_layout = 
    {
	8, 8,
	256,
	2,
	{0, 0x4000},
	{gfx_step8(0, 1)},
	{gfx_step8(0, 8)},
	64
    };

    static BerrnGfxLayout sprite_layout = 
    {
	16, 16,
	64,
	2,
	{0, 0x4000},
	{gfx_step8(0, 1), gfx_step8(64, 1)},
	{gfx_step8(0, 8), gfx_step8(128, 8)},
	256
    };

    galaxianvideo::galaxianvideo(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(256, 224);
	bitmap->clear();
    }

    galaxianvideo::~galaxianvideo()
    {

    }

    void galaxianvideo::init()
    {
	pal_rom = driver.get_rom_region("pal");
	tile_rom = driver.get_rom_region("gfx");
	tile_ram.resize((256 * 8 * 8 * 2), 0);
	gfxDecodeSet(char_layout, tile_rom, tile_ram);
	video_ram.fill(0);
	obj_ram.fill(0);
    }

    void galaxianvideo::shutdown()
    {
	pal_rom.clear();
	tile_rom.clear();
	tile_ram.clear();
	bitmap->clear();
    }

    void galaxianvideo::updatePixels()
    {
	bitmap->fillcolor(black());

	for (int col = 0; col < 32; col++)
	{
	    for (int row = 0; row < 32; row++)
	    {
		int offs = ((32 * col) + row);

		uint8_t scroll = obj_ram.at(row * 2);
		uint32_t color = (obj_ram.at((row * 2) + 1) & 7);

		int cx = (row * 8);
		int cy = ((col * 8) - 16);
		if (inRange(cy, -7, 224))
		{
		    uint8_t tile_num = video_ram.at(offs);
		    draw_tile(tile_num, cx, cy, color, scroll);
		}
	    }
	}

	driver.set_screen(bitmap);
    }

    void galaxianvideo::draw_tile(uint32_t tile_num, int xcoord, int ycoord, int pal_num, int scroll)
    {
	if (!inRange(xcoord, 0, 256) || !inRange(ycoord, 0, 224))
	{
	    return;
	}

	int base_x = xcoord;
	int base_y = ycoord;
	int color_offs = (pal_num & 0x7);

	for (int index = 0; index < 64; index++)
	{
	    int py = (index / 8);
	    int px = (index % 8);

	    uint8_t tile_color = tile_ram.at((tile_num * 64) + index);
	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    ypos = ((ypos - scroll) % 256);

	    uint8_t color = pal_rom.at((color_offs * 4) + tile_color);
	    set_pixel(xpos, ypos, color);
	}
    }

    void galaxianvideo::set_pixel(int xpos, int ypos, uint8_t color)
    {
	if (color == 0)
	{
	    return;
	}

	bool red0 = testbit(color, 0);
	bool red1 = testbit(color, 1);
	bool red2 = testbit(color, 2);

	bool green0 = testbit(color, 3);
	bool green1 = testbit(color, 4);
	bool green2 = testbit(color, 5);

	bool blue0 = testbit(color, 6);
	bool blue1 = testbit(color, 7);

	int red = ((red0 * 0x1D) + (red1 * 0x3E) + (red2 * 0x85));
	int green = ((green0 * 0x1D) + (green1 * 0x3E) + (green2 * 0x85));
	int blue = ((blue0 * 0x47) + (blue1 * 0x99));

	bitmap->setPixel(xpos, ypos, fromRGB(red, green, blue));
    }

    uint8_t galaxianvideo::readVRAM(int addr)
    {
	addr &= 0x3FF;
	return video_ram.at(addr);
    }

    void galaxianvideo::writeVRAM(int addr, uint8_t data)
    {
	addr &= 0x3FF;
	video_ram.at(addr) = data;
    }

    uint8_t galaxianvideo::readORAM(int addr)
    {
	addr &= 0xFF;
	return obj_ram.at(addr);
    }

    void galaxianvideo::writeORAM(int addr, uint8_t data)
    {
	addr &= 0xFF;
	obj_ram.at(addr) = data;
    }

    void galaxianvideo::writeIO(int reg, bool line)
    {
	switch (reg)
	{
	    case 0: is_starfield_enabled = line; break;
	    default: break;
	}
    }
};