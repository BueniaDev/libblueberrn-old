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
	init_starfield();
    }

    void galaxianvideo::init_starfield()
    {
	// Initial setup derived from MAME

	int stars = 0;
	uint32_t generator = 0;

	for (int ypos = 0; ypos < 512; ypos++)
	{
	    for (int xpos = 0; xpos < 256; xpos++)
	    {
		bool gen_bit = (!testbit(generator, 16) != testbit(generator, 4));
		generator = ((generator << 1) | gen_bit);

		if (!testbit(generator, 16) && ((generator & 0xFF) == 0xFF))
		{
		    int color = ((~(generator >> 8)) & 0x3F);

		    if (color != 0)
		    {
			if (stars < 2520)
			{
			    starfield[stars].xpos = xpos;
			    starfield[stars].ypos = ypos;
			    starfield[stars].color = color;
			    stars += 1;
			}
		    }
		}
	    }
	}
    }

    void galaxianvideo::update_star_scroll()
    {
	if (is_starfield_enabled)
	{
	    starfield_scroll_pos += 1;
	}
	else
	{
	    starfield_scroll_pos = 0;
	}
    }

    void galaxianvideo::draw_star_pixel(int xpos, int ypos, int color)
    {
	color &= 0x3F;

	array<uint8_t, 4> starmap = {0, 194, 214, 255};

	int red = starmap.at((color & 0x3));
	int green = starmap.at(((color >> 2) & 0x3));
	int blue = starmap.at(((color >> 4) & 0x3));

	bitmap->setPixel(xpos, ypos, fromRGB(red, green, blue));
    }

    void galaxianvideo::draw_starfield()
    {
	for (int i = 0; i < 2520; i++)
	{
	    int ypos = (starfield[i].ypos + starfield_scroll_pos);
	    int xpos = ((starfield[i].xpos + (ypos >> 9)) & 0xFF);

	    ypos = ((ypos & 0x1FF) / 2);

	    if (testbit(xpos, 0) != testbit(ypos, 3))
	    {
		draw_star_pixel(ypos, (xpos - 16), starfield[i].color);
	    }
	}
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
	update_star_scroll();
	bitmap->fillcolor(black());

	if (is_starfield_enabled)
	{
	    draw_starfield();
	}

	for (int col = 0; col < 32; col++)
	{
	    for (int row = 0; row < 32; row++)
	    {
		int offs = ((32 * col) + row);

		uint8_t scroll = obj_ram.at(row * 2);
		uint32_t color = (obj_ram.at((row * 2) + 1) & 7);

		int cx = (row * 8);
		int cy = ((col * 8) - 16);
		
		uint8_t tile_num = video_ram.at(offs);
		draw_tile(tile_num, cx, cy, color, scroll);
	    }
	}

	for (int y = 0; y < 224; y++)
	{
	    uint8_t shell = 0xFF;
	    uint8_t missile = 0xFF;
	    uint8_t y_eff = 0;

	    y_eff = (y + 15);

	    for (int i = 0; i < 3; i++)
	    {
		uint32_t bullet_offs = (0x60 + (i * 4));
		uint8_t pos = uint8_t(obj_ram.at(bullet_offs + 1) + y_eff);

		if (pos == 0xFF)
		{
		    shell = i;
		}
	    }

	    y_eff = (y + 16);

	    for (int i = 3; i < 8; i++)
	    {
		uint32_t bullet_offs = (0x60 + (i * 4));
		uint8_t pos = uint8_t(obj_ram.at(bullet_offs + 1) + y_eff);

		if (pos == 0xFF)
		{
		    if (i != 7)
		    {
			shell = i;
		    }
		    else
		    {
			missile = i;
		    }
		}
	    }

	    if (shell != 0xFF)
	    {
		uint32_t bullet_offs = (0x60 + (shell * 4));
		uint32_t ypos = y;
		uint32_t xpos = (255 - obj_ram.at(bullet_offs + 3));

		xpos -= 4;

		for (int i = 0; i < 4; i++)
		{
		    bitmap->setPixel(xpos, ypos, white());
		    xpos += 1;
		}
	    }

	    if (missile != 0xFF)
	    {
		uint32_t bullet_offs = (0x60 + (missile * 4));
		uint32_t ypos = y;
		uint32_t xpos = (255 - obj_ram.at(bullet_offs + 3));

		xpos -= 4;

		for (int i = 0; i < 4; i++)
		{
		    bitmap->setPixel(xpos, ypos, yellow());
		    xpos += 1;
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

	    if (ypos <= -8)
	    {
		ypos += 256;
	    }

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