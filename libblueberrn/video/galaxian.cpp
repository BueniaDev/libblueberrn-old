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
	bitmap = new BerrnBitmapRGB(224, 256);
	bitmap->clear();
    }

    galaxianvideo::~galaxianvideo()
    {

    }

    bool galaxianvideo::init()
    {
	pal_rom = driver.get_rom_region("pal");
	tile_rom = driver.get_rom_region("gfx");
	tile_ram.resize((256 * 8 * 8 * 2), 0);
	sprite_ram.resize((64 * 16 * 16 * 2), 0);
	gfxDecodeSet(char_layout, tile_rom, tile_ram);
	gfxDecodeSet(sprite_layout, tile_rom, sprite_ram);
	video_ram.fill(0);
	obj_ram.fill(0);
	init_starfield();
	return true;
    }

    void galaxianvideo::shutdown()
    {
	pal_rom.clear();
	tile_rom.clear();
	sprite_ram.clear();
	tile_ram.clear();
	bitmap->clear();
    }

    void galaxianvideo::init_starfield()
    {
	// Initial setup derived from the MAME driver in galaxian.c(pp)

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
	if (is_stars_enabled)
	{
	    starfield_scroll_pos += 1;
	}
	else
	{
	    starfield_scroll_pos = 0;
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

    void galaxianvideo::draw_tile(uint8_t tile_num, int x, int y, int pal_num)
    {
	if (!inRange(x, 0, 224) || !inRange(y, 0, 256))
	{
	    return;
	}

	int color_offs = (pal_num & 0x7);

	for (int index = 0; index < 64; index++)
	{
	    int py = (index % 8);
	    int px = (7 - (index / 8));

	    uint8_t tile_color = tile_ram.at((tile_num * 64) + index);
	    int ypos = (y + py);
	    int xpos = (x + px);
	    uint8_t color = pal_rom.at((color_offs * 4) + tile_color);
	    set_pixel(xpos, ypos, color);
	}
    }

    void galaxianvideo::draw_sprite(uint8_t sprite_num, int x, int y, bool flip_x, bool flip_y, int pal_num)
    {
	if (!inRange(x, 0, 224) || !inRange(y, 0, 256))
	{
	    return;
	}

	int color_offs = (pal_num & 0x7);

	for (int index = 0; index < 256; index++)
	{
	    int py = (index % 16);
	    int px = (15 - (index / 16));

	    uint8_t sprite_color = sprite_ram.at((sprite_num * 256) + index);

	    if (flip_x)
	    {
		px = (15 - px);
	    }

	    if (flip_y)
	    {
		py = (15 - py);
	    }

	    int xpos = (x + px);
	    int ypos = (y + py);

	    if (!inRange(xpos, 0, 224))
	    {
		continue;
	    }

	    uint8_t color = pal_rom.at((color_offs * 4) + sprite_color);
	    set_pixel(xpos, ypos, color);
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
	    int ypos = starfield[i].ypos + starfield_scroll_pos;
	    int xpos = ((starfield[i].xpos + (ypos >> 9)) & 0xFF);

	    ypos = ((ypos & 0x1FF) / 2);

	    if (testbit(xpos, 0) != testbit(ypos, 3))
	    {
		draw_star_pixel((xpos - 16), ypos, starfield[i].color);
	    }
	}
    }

    void galaxianvideo::update_pixels()
    {
	update_star_scroll();
	bitmap->fillcolor(black());

	if (is_stars_enabled)
	{
	    draw_starfield();
	}

	for (int y = 0; y < 32; y++)
	{
	    uint8_t scroll = obj_ram[(y * 2)];
	    uint32_t color = (obj_ram[(y * 2) + 1] & 7);

	    for (int x = 0; x < 32; x++)
	    {
		uint32_t offset = (y + ((31 - x) * 32));

		int cx = ((((x * 8) + scroll) & 0xFF) - 16);
		int cy = (y * 8);

		if (inRange(cx, -7, 224))
		{
		    uint8_t char_code = video_ram[offset];
		    draw_tile(char_code, cx, cy, color);
		}
	    }
	}

	for (int bullet = 0; bullet < 8; bullet++)
	{
	    uint32_t bullet_offs = (0x60 + (bullet * 4));
	    int xpos = (obj_ram.at(bullet_offs + 1) - 16);
	    int ypos = (255 - obj_ram.at(bullet_offs + 3));

	    berrnRGBA bullet_color = (bullet == 7) ? yellow() : white();

	    for (int index = 0; index < 4; index++)
	    {
		ypos -= 1;

		if (ypos >= 0)
		{
		    bitmap->setPixel(xpos, ypos, bullet_color);
		}
	    }
	}

	for (int sprite = 7; sprite >= 0; sprite--)
	{
	    uint32_t sprite_offs = (0x40 + (sprite * 4));

	    int ypos = (obj_ram.at(sprite_offs + 3) + 1);
	    int xpos = (obj_ram.at(sprite_offs) - 16);

	    if (ypos < 8)
	    {
		continue;
	    }

	    uint8_t flip_attrib = (obj_ram.at(sprite_offs + 1));

	    bool is_yflip = testbit(flip_attrib, 6);
	    bool is_xflip = testbit(flip_attrib, 7);

	    if (sprite <= 2)
	    {
		xpos += 1;
	    }

	    if (!inRange(xpos, -16, 224))
	    {
		continue;
	    }

	    uint8_t sprite_num = (flip_attrib & 0x3F);

	    int color = (obj_ram.at(sprite_offs + 2) & 0x7);
	    draw_sprite(sprite_num, xpos, ypos, is_xflip, is_yflip, color);
	}

	driver.setScreen(bitmap);
    }

    uint8_t galaxianvideo::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0xFFF;

	if (addr < 0x800)
	{
	    data = video_ram[(addr & 0x3FF)];
	}
	else
	{
	    data = obj_ram[(addr & 0xFF)];
	}

	return data;
    }

    void galaxianvideo::writeByte(uint16_t addr, uint8_t data)
    {
	addr &= 0xFFF;

	if (addr < 0x800)
	{
	    video_ram[(addr & 0x3FF)] = data;
	}
	else
	{
	    obj_ram[(addr & 0xFF)] = data;
	}
    }

    void galaxianvideo::update_latch(int addr, uint8_t data)
    {
	addr &= 3;
	bool val = testbit(data, 0);

	switch (addr)
	{
	    case 0: is_stars_enabled = val; break;
	    case 1: is_flip_screenx = val; break;
	    case 2: is_flip_screeny = val; break;
	}
    }
};