/*
    This file is part of libblueberrn.
    Copyright (C) 2021 BueniaDev.

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
	bitmap = new BerrnBitmapRGB(224, 288);
	bitmap->clear();
    }

    pacmanvideo::~pacmanvideo()
    {

    }

    bool pacmanvideo::init()
    {
	col_rom = driver.get_rom_region("color");
	pal_rom = driver.get_rom_region("pal");
	tile_rom = driver.get_rom_region("gfx1");
	sprite_rom = driver.get_rom_region("gfx2");
	tile_ram.resize((256 * 8 * 8), 0);
	sprite_ram.resize((64 * 16 * 16), 0);

	gfxDecodeSet(charlayout, tile_rom, tile_ram);
	gfxDecodeSet(spritelayout, sprite_rom, sprite_ram);

	vram.fill(0);
	cram.fill(0);
	obj_ram.fill(0);
	obj_pos.fill(0);
	return true;
    }

    void pacmanvideo::shutdown()
    {
	bitmap->clear();
    }

    void pacmanvideo::updatePixels()
    {
	update_tiles();
	update_sprites();
    }

    void pacmanvideo::update_tiles()
    {
	for (int addr = 0; addr < 64; addr++)
	{
	    int ypos = (34 + (addr / 32));
	    int xpos = (31 - (addr % 32));

	    uint8_t tile_num = vram[addr];
	    uint8_t pal_num = cram[addr];
	    draw_tile(tile_num, pal_num, xpos, ypos);
	}

	for (int addr = 0; addr < 0x380; addr++)
	{
	    int ypos = (2 + (addr % 32));
	    int xpos = (29 - (addr / 32));

	    uint8_t tile_num = vram[(0x40 + addr)];
	    uint8_t pal_num = cram[(0x40 + addr)];
	    draw_tile(tile_num, pal_num, xpos, ypos);
	}

	for (int addr = 0; addr < 64; addr++)
	{
	    int ypos = (addr / 32);
	    int xpos = (31 - (addr % 32));

	    uint8_t tile_num = vram[(0x3C0 + addr)];
	    uint8_t pal_num = cram[(0x3C0 + addr)];
	    draw_tile(tile_num, pal_num, xpos, ypos);
	}
    }

    void pacmanvideo::update_sprites()
    {
	for (int sprite = 7; sprite >= 0; sprite--)
	{
	    int16_t xpos = (224 - obj_pos[(sprite * 2)] + 15);
	    int16_t ypos = (288 - obj_pos[((sprite * 2) + 1)] - 16);

	    uint8_t sprite_info = obj_ram[(sprite * 2)];
	    uint8_t pal_num = obj_ram[((sprite * 2) + 1)];

	    bool xflip = testbit(sprite_info, 1);
	    bool yflip = testbit(sprite_info, 0);

	    uint8_t sprite_num = (sprite_info >> 2);
	    draw_sprite(sprite_num, pal_num, xpos, ypos, xflip, yflip);
	}
    }

    void pacmanvideo::set_pixel(int xpos, int ypos, uint8_t color_num)
    {
	if (!inRange(xpos, 0, 224) || !inRange(ypos, 0, 288))
	{
	    return;
	}

	uint8_t color = col_rom[color_num];

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

    void pacmanvideo::draw_tile(int tile_num, int pal_num, int xcoord, int ycoord)
    {
	int base_x = ((xcoord - 2) * 8);
	int base_y = (ycoord * 8);
	int pal_offs = (pal_num & 0x3F);

	if (!inRange(base_x, 0, 224))
	{
	    return;
	}

	for (int pixel = 0; pixel < 64; pixel++)
	{
	    int py = (pixel % 8);
	    int px = (7 - (pixel / 8));

	    uint8_t color_num = tile_ram[((tile_num * 64) + pixel)];
	    int xpos = (base_x + px);
	    int ypos = (base_y + py);
	    uint8_t color = pal_rom[((pal_offs * 4) + color_num)];
	    set_pixel(xpos, ypos, color);
	}
    }

    void pacmanvideo::draw_sprite(int sprite_num, int pal_num, int xcoord, int ycoord, bool flipx, bool flipy)
    {
	if (!inRange(xcoord, -16, 224))
	{
	    return;
	}

	int pal_offs = (pal_num & 0x3F);

	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 256; pixel++)
	{
	    int py = (pixel % 16);
	    int px = (15 - (pixel / 16));

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

	    if (!inRange(xpos, 0, 224))
	    {
		continue;
	    }

	    uint8_t color_num = sprite_ram[((sprite_num * 256) + pixel)];

	    uint8_t color = pal_rom[((pal_offs * 4) + color_num)];

	    if (color == 0)
	    {
		continue;
	    }

	    set_pixel(xpos, ypos, color);
	}
    }

    uint8_t pacmanvideo::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x7FF;

	if (addr < 0x400)
	{
	    data = vram[(addr & 0x3FF)];
	}
	else
	{
	    data = cram[(addr & 0x3FF)];
	}

	return data;
    }

    void pacmanvideo::writeByte(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FF;

	if (addr < 0x400)
	{
	    vram[(addr & 0x3FF)] = data;
	}
	else
	{
	    cram[(addr & 0x3FF)] = data;
	}
    }

    uint8_t pacmanvideo::readSprites(int addr)
    {
	addr &= 0xF;
	return obj_ram[addr];
    }

    void pacmanvideo::writeSprites(int addr, uint8_t data)
    {
	addr &= 0xF;
	obj_ram[addr] = data;
    }

    void pacmanvideo::writeSpritePos(int addr, uint8_t data)
    {
	addr &= 0xF;
	obj_pos[addr] = data;
    }
};