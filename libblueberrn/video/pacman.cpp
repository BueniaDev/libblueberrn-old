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

#include "pacman.h"
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
	64,
	2,
	{0, 4},
	{ gfx_step4(64, 1), gfx_step4(128, 1), gfx_step4(192, 1), gfx_step4(0, 1) },
	{ gfx_step8(0, 8), gfx_step8(256, 8) },
	512
    };

    pacmanvideo::pacmanvideo(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(288, 224);
	bitmap->clear();
    }

    pacmanvideo::~pacmanvideo()
    {

    }

    void pacmanvideo::init()
    {
	color_rom = driver.get_rom_region("color");
	pal_rom = driver.get_rom_region("pal");
	auto tile_rom = driver.get_rom_region("gfx1");
	auto sprite_rom = driver.get_rom_region("gfx2");

	tile_ram.resize((256 * 8 * 8), 0);
	sprite_ram.resize((64 * 16 * 16), 0);

	gfxDecodeSet(charlayout, tile_rom, tile_ram);
	gfxDecodeSet(spritelayout, sprite_rom, sprite_ram);

	video_ram.fill(0);
	color_ram.fill(0);
	obj_ram[0].fill(0);
	obj_ram[1].fill(0);
    }

    void pacmanvideo::shutdown()
    {
	bitmap->clear();
	tile_ram.clear();
	sprite_ram.clear();
    }

    uint32_t pacmanvideo::tilemap_scan(uint32_t row, uint32_t col)
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

    void pacmanvideo::updatePixels()
    {
	// Draw tiles

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

	for (int sprite = 7; sprite >= 0; sprite--)
	{
	    int ypos = (obj_ram[1][(sprite * 2)] - 31);
	    int xpos = (272 - obj_ram[1][((sprite * 2) + 1)]);

	    uint8_t sprite_info = obj_ram[0][(sprite * 2)];
	    uint8_t pal_num = obj_ram[0][((sprite * 2) + 1)];

	    bool xflip = testbit(sprite_info, 0);
	    bool yflip = testbit(sprite_info, 1);

	    uint8_t sprite_num = (sprite_info >> 2);
	    draw_sprite(sprite_num, xpos, ypos, pal_num, xflip, yflip);
	}

	driver.set_screen(bitmap);
    }

    void pacmanvideo::draw_tile(uint32_t tile_num, int xcoord, int ycoord, int pal_num)
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

	    uint8_t color = pal_rom.at((pal_num * 4) + color_num);
	    set_pixel(xpos, ypos, color);
	}
    }

    void pacmanvideo::draw_sprite(uint32_t sprite_num, int xcoord, int ycoord, int pal_num, bool flipx, bool flipy)
    {
	if (!inRange(ycoord, -16, 224))
	{
	    return;
	}

	pal_num &= 0x3F;

	int base_x = xcoord;
	int base_y = ycoord;

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

	    if (!inRange(ypos, 0, 224))
	    {
		continue;
	    }

	    uint8_t color_num = sprite_ram.at(((sprite_num * 256) + pixel));

	    uint8_t color = pal_rom.at(((pal_num * 4) + color_num));

	    if (color == 0)
	    {
		continue;
	    }

	    set_pixel(xpos, ypos, color);
	}
    }

    void pacmanvideo::set_pixel(int xpos, int ypos, uint8_t color_num)
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

    uint8_t pacmanvideo::readVRAM(uint16_t addr)
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

    void pacmanvideo::writeVRAM(uint16_t addr, uint8_t data)
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

    uint8_t pacmanvideo::readORAM(uint16_t addr)
    {
	addr &= 0xF;
	return obj_ram[0][addr];
    }

    void pacmanvideo::writeORAM(int bank, uint16_t addr, uint8_t data)
    {
	if (!inRange(bank, 0, 2))
	{
	    throw out_of_range("Invalid sprite RAM bank");
	}

	addr &= 0xF;
	obj_ram[bank][addr] = data;
    }
};