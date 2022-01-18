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

    berrn1942video::berrn1942video(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(224, 256);
	bitmap->clear();
    }

    berrn1942video::~berrn1942video()
    {
	bitmap = NULL;
    }

    void berrn1942video::init_palette()
    {
	auto color_prom = driver.get_rom_region("palproms");

	for (int i = 0; i < 256; i++)
	{
	    berrnRGBA color = black();
	    for (int j = 0; j < 3; j++)
	    {
		bool bit0 = testbit(color_prom.at(i + (j * 256)), 0);
		bool bit1 = testbit(color_prom.at(i + (j * 256)), 1);
		bool bit2 = testbit(color_prom.at(i + (j * 256)), 2);
		bool bit3 = testbit(color_prom.at(i + (j * 256)), 3);

		int color_val = ((bit0 * 0x0E) + (bit1 * 0x1F) + (bit2 * 0x43) + (bit3 * 0x8F));
		color.at(j) = color_val;
	    }

	    colors.at(i) = color;
	}
    }

    bool berrn1942video::init()
    {
	bg_vram.fill(0);
	fg_vram.fill(0);
	sprite_ram.fill(0);

	for (int i = 0; i < 4; i++)
	{
	    bg_pal.at(i).fill(0);
	}

	fg_pal.fill(0);

	fg_rom = driver.get_rom_region("gfx1");
	bg_rom = driver.get_rom_region("gfx2");
	bg_ram.resize((512 * 16 * 16), 0);
	fg_ram.resize((512 * 8 * 8), 0);
	gfxDecodeSet(bg_layout, bg_rom, bg_ram);
	gfxDecodeSet(fg_layout, fg_rom, fg_ram);
	palette_bank = 0;
	scroll_x = 0;

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

	init_palette();

	return true;
    }

    void berrn1942video::shutdown()
    {
	bg_rom.clear();
	fg_rom.clear();
	
	bg_ram.clear();
	fg_ram.clear();
	bitmap->clear();
    }

    void berrn1942video::update_pixels()
    {
	update_background();
	update_foreground();
	driver.setScreen(bitmap);
    }

    void berrn1942video::update_background()
    {
	for (int col = 0; col < 32; col++)
	{
	    for (int row = 0; row < 16; row++)
	    {
		int offs = ((32 * (31 - col)) + row);

		uint32_t tile_num = bg_vram.at(offs);
		uint32_t color_attrib = bg_vram.at(offs + 0x10);

		bool flipx = testbit(color_attrib, 6);
		bool flipy = testbit(color_attrib, 5);

		tile_num |= (testbit(color_attrib, 7) << 8);

		int xpos = ((row * 16) - 16);
		int ypos = (col * 16);

		ypos -= 256;
		ypos += scroll_x;

		// Wrap around ypos value if needed
		while (ypos >= 512)
		{
		    ypos -= 512;
		}

		uint32_t pal_num = ((color_attrib & 0x1F) | (0x20 * palette_bank));

		if (ypos > (512 - 16))
		{
		    draw_bg_tile(tile_num, pal_num, xpos, (ypos - 512), flipx, flipy);
		}

		draw_bg_tile(tile_num, pal_num, xpos, ypos, flipx, flipy);
	    }
	}
    }

    void berrn1942video::update_foreground()
    {
	for (int row = 0; row < 32; row++)
	{
	    for (int col = 0; col < 32; col++)
	    {
		uint32_t offset = ((32 * row) + (31 - col));

		uint32_t tile_num = fg_vram.at(offset);
		uint32_t color_attrib = fg_vram.at(offset + 0x400);

		tile_num |= (testbit(color_attrib, 7) << 8);

		uint32_t color_num = (color_attrib & 0x3F);

		int xpos = ((row * 8) - 16);
		int ypos = (col * 8);

		draw_fg_tile(tile_num, color_num, xpos, ypos);
	    }
	}
    }

    void berrn1942video::draw_fg_tile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord)
    {
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 64; pixel++)
	{
	    int py = (7 - (pixel % 8));
	    int px = (pixel / 8);

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    int color_val = (pal_num & 0x3F);

	    uint8_t color_num = fg_ram.at((tile_num * 64) + pixel);

	    if (color_num == 0)
	    {
		continue;
	    }

	    uint8_t color = fg_pal.at((color_val * 4) + color_num);

	    bitmap->setPixel(xpos, ypos, colors.at(color));
	}
    }

    void berrn1942video::draw_bg_tile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord, bool xflip, bool yflip)
    {
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 256; pixel++)
	{
	    int py = (15 - (pixel % 16));
	    int px = (pixel / 16);

	    if (yflip)
	    {
		py = (15 - py);
	    }

	    if (xflip)
	    {
		px = (15 - px);
	    }

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    int color_val = (pal_num & 0x1F);
	    int pal_bank = ((pal_num >> 5) & 0x3);

	    auto tile_color = bg_pal.at(pal_bank);

	    uint8_t color_num = bg_ram.at((tile_num * 256) + pixel);
	    uint8_t color = tile_color.at(((color_val * 8) + color_num));
	    bitmap->setPixel(xpos, ypos, colors.at(color));
	}
    }

    uint8_t berrn1942video::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0xFFF;

	if (inRange(addr, 0, 0x800))
	{
	    data = fg_vram.at(addr);
	}
	else if (inRange(addr, 0x800, 0xC00))
	{
	    data = bg_vram.at((addr & 0x3FF));
	}

	return data;
    }

    void berrn1942video::writeByte(uint16_t addr, uint8_t data)
    {
	addr &= 0xFFF;

	if (inRange(addr, 0, 0x800))
	{
	    fg_vram.at(addr) = data;
	}
	else if (inRange(addr, 0x800, 0xC00))
	{
	    bg_vram.at((addr & 0x3FF)) = data;
	}
    }

    uint8_t berrn1942video::readSprites(uint16_t addr)
    {
	addr &= 0x7F;
	return sprite_ram.at(addr);
    }

    void berrn1942video::writeSprites(uint16_t addr, uint8_t data)
    {
	addr &= 0x7F;
	sprite_ram.at(addr) = data;
    }

    void berrn1942video::setScroll(bool is_msb, uint8_t data)
    {
	if (is_msb)
	{
	    scroll_x = ((scroll_x & 0xFF) | (data << 8));
	}
	else
	{
	    scroll_x = ((scroll_x & 0xFF00) | data);
	}
    }

    void berrn1942video::setPaletteBank(uint8_t data)
    {
	palette_bank = (data & 3);
    }
};