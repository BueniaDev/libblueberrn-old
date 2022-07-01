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

#include "galaga.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout charlayout = 
    {
	8, 8,
	256,
	2,
	{0, 4},
	{ gfx_step4(64, 1), gfx_step4(0, 1) },
	{ gfx_step8(0, 8) },
	128
    };

    static BerrnGfxLayout spritelayout =
    {
	16, 16,
	128,
	2,
	{0, 4},
	{ gfx_step4(0, 1), gfx_step4(64, 1), gfx_step4(128, 1), gfx_step4(192, 1) },
	{ gfx_step8(0, 8), gfx_step8(256, 8) },
	512
    };

    galagavideo::galagavideo(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(288, 224);
	bitmap->clear();
    }

    galagavideo::~galagavideo()
    {

    }

    void galagavideo::init()
    {
	video_ram.fill(0);
	color_ram.fill(0);

	for (int i = 0; i < 3; i++)
	{
	    sprite_ram.at(i).fill(0);
	}

	color_rom = driver.get_rom_region("color");
	tile_pal_rom = driver.get_rom_region("tilepal");

	for (size_t index = 0; index < tile_pal_rom.size(); index++)
	{
	    tile_pal_rom.at(index) = ((tile_pal_rom.at(index) & 0xF) | 0x10);
	}

	auto tile_rom = driver.get_rom_region("gfx1");

	gfxDecodeSet(charlayout, tile_rom, tile_ram);

	
    }

    void galagavideo::shutdown()
    {
	bitmap->clear();
	color_rom.clear();
	tile_pal_rom.clear();
	tile_ram.clear();
    }

    uint32_t galagavideo::tilemap_scan(uint32_t row, uint32_t col)
    {
	uint32_t offs = 0;

	row += 2;
	col -= 2;

	if (testbit(col, 5))
	{
	    offs = row + ((col & 0x1F) * 32);
	}
	else
	{
	    offs = col + (row * 32);
	}

	return offs;
    }

    void galagavideo::updatePixels()
    {
	bitmap->fillcolor(black());
	for (int col = 0; col < 36; col++)
	{
	    for (int row = 0; row < 28; row++)
	    {
		uint32_t offset = tilemap_scan(row, col);
		uint8_t tile_number = video_ram.at(offset);
		uint8_t color_number = (color_ram.at(offset) & 0x3F);

		int xpos = (col * 8);
		int ypos = (row * 8);
		draw_tile(tile_number, xpos, ypos, color_number);
	    }
	}

	driver.set_screen_bmp(bitmap);
    }

    void galagavideo::draw_tile(uint32_t tile_num, int xcoord, int ycoord, int pal_num)
    {
	if (!inRange(xcoord, 0, 288) || !inRange(ycoord, 0, 224))
	{
	    return;
	}

	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 64; pixel++)
	{
	    int py = (pixel / 8);
	    int px = (pixel % 8);

	    uint8_t color_num = tile_ram.at((tile_num * 64) + pixel);
	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    uint8_t color = tile_pal_rom.at((pal_num * 4) + color_num);

	    if (color == 0x1F)
	    {
		continue;
	    }

	    set_pixel(xpos, ypos, color);
	}
    }

    void galagavideo::set_pixel(int xpos, int ypos, uint8_t color_num)
    {
	uint8_t color = color_rom.at(color_num);

	bool red0 = testbit(color, 0);
	bool red1 = testbit(color, 1);
	bool red2 = testbit(color, 2);

	bool green0 = testbit(color, 3);
	bool green1 = testbit(color, 4);
	bool green2 = testbit(color, 5);

	bool blue0 = testbit(color, 6);
	bool blue1 = testbit(color, 7);

	int red = ((red0 * 0x21) + (red1 * 0x47) + (red2 * 0x97));
	int green = ((green0 * 0x21) + (green1 * 0x47) + (green2 * 0x97));
	int blue = ((blue0 * 0x51) + (blue1 * 0xAE));

	bitmap->setPixel(xpos, ypos, fromRGB(red, green, blue));
    }

    uint8_t galagavideo::readVRAM(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x7FF;

	if (addr < 0x400)
	{
	    data = video_ram.at(addr);
	}
	else
	{
	    data = color_ram.at(addr & 0x3FF);
	}

	return data;
    }

    void galagavideo::writeVRAM(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FF;
	if (addr < 0x400)
	{
	    video_ram.at(addr) = data;
	}
	else
	{
	    color_ram.at(addr & 0x3FF) = data;
	}
    }

    uint8_t galagavideo::readORAM(int bank, uint16_t addr)
    {
	uint8_t data = 0;
	bank &= 3;
	addr &= 0x7F;

	if (bank != 3)
	{
	    data = sprite_ram[bank][addr];
	}

	return data;
    }

    void galagavideo::writeORAM(int bank, uint16_t addr, uint8_t data)
    {
	bank &= 3;
	addr &= 0x7F;

	if (bank != 3)
	{
	    sprite_ram[bank][addr] = data;
	}
    }
};