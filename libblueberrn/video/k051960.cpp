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

#include "k051960.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    k051960video::k051960video(berrndriver &drv) : driver(drv)
    {

    }

    k051960video::~k051960video()
    {

    }

    void k051960video::init()
    {
	obj_ram.fill(0);
	shadow_config = 0;
    }

    void k051960video::setROM(vector<uint8_t> &rom)
    {
	obj_rom = vector<uint8_t>(rom.begin(), rom.end());
    }

    void k051960video::setTiles(vector<uint8_t> &tiles)
    {
	obj_tiles = vector<uint8_t>(tiles.begin(), tiles.end());
    }

    void k051960video::setSpriteCallback(k051960callback cb)
    {
	spritecb = cb;
    }

    void k051960video::shutdown()
    {
	obj_tiles.clear();
    }

    void k051960video::render(int min_priority, int max_priority)
    {
	// TODO: Implement the following features:
	// Priority blending
	// Sprite zooming

	(void)min_priority;
	(void)max_priority;

	obj_buffer.fill(0);

	array<int, 8> xoffset = {0, 1, 4, 5, 16, 17, 20, 21};
	array<int, 8> yoffset = {0, 2, 8, 10, 32, 34, 40, 42};
	array<int, 8> width = {1, 2, 1, 2, 4, 2, 4, 8};
	array<int, 8> height = {1, 1, 2, 2, 2, 4, 4, 8};

	array<int, 128> sortedlist;
	sortedlist.fill(-1);

	for (int offs = 0; offs < 0x400; offs += 8)
	{
	    uint8_t prior_active = obj_ram.at(offs);
	    int sprite_priority = (prior_active & 0x7F);

	    if (testbit(prior_active, 7))
	    {
		sortedlist.at(sprite_priority) = offs;
	    }
	}

	for (int sprite = 0; sprite < 128; sprite++)
	{
	    int sprite_offs = sortedlist.at(sprite);

	    if (sprite_offs == -1)
	    {
		continue;
	    }

	    uint8_t attrib1 = obj_ram.at(sprite_offs + 1);
	    uint8_t attrib2 = obj_ram.at(sprite_offs + 2);
	    uint8_t attrib3 = obj_ram.at(sprite_offs + 3);
	    uint8_t attrib4 = obj_ram.at(sprite_offs + 4);
	    uint8_t attrib5 = obj_ram.at(sprite_offs + 5);
	    uint8_t attrib6 = obj_ram.at(sprite_offs + 6);
	    uint8_t attrib7 = obj_ram.at(sprite_offs + 7);

	    uint16_t sprite_num = (((attrib1 & 0x1F) << 8) | attrib2);
	    uint8_t color_attrib = attrib3;
	    uint8_t priority = 0;

	    bool is_shadow = false;

	    if (testbit(shadow_config, 2))
	    {
		is_shadow = false;
	    }
	    else if (testbit(shadow_config, 1))
	    {
		is_shadow = true;
	    }
	    else
	    {
		is_shadow = testbit(color_attrib, 7);
	    }

	    if (spritecb)
	    {
		spritecb(sprite_num, color_attrib, priority, is_shadow);
	    }

	    int size = ((attrib1 >> 5) & 0x7);
	    int obj_width = width.at(size);
	    int obj_height = height.at(size);

	    if (obj_width >= 2)
	    {
		sprite_num &= ~0x01;
	    }

	    if (obj_height >= 2)
	    {
		sprite_num &= ~0x02;
	    }

	    if (obj_width >= 4)
	    {
		sprite_num &= ~0x04;
	    }

	    if (obj_height >= 4)
	    {
		sprite_num &= ~0x08;
	    }

	    if (obj_width >= 8)
	    {
		sprite_num &= ~0x10;
	    }

	    if (obj_height >= 8)
	    {
		sprite_num &= ~0x20;
	    }

	    int sprite_xpos = ((testbit(attrib6, 0) << 8) | attrib7);
	    int sprite_ypos = (256 - ((testbit(attrib4, 0) << 8) | attrib5));

	    bool is_flipx = testbit(attrib6, 1);
	    bool is_flipy = testbit(attrib4, 1);

	    int32_t xzoom = 0x10000;
	    int32_t yzoom = 0x10000;

	    int width_mask = (obj_width - 1);
	    int height_mask = (obj_height - 1);

	    if ((xzoom == 0x10000) && (yzoom == 0x10000))
	    {
		for (int sy = 0; sy < obj_height; sy++)
		{
		    int ycoord = (sprite_ypos + (16 * sy));

		    for (int sx = 0; sx < obj_width; sx++)
		    {
			int32_t sprite_code = sprite_num;

			int xcoord = (sprite_xpos + (16 * sx));

			int xoffs = sx;
			int yoffs = sy;

			if (is_flipx)
			{
			    xoffs = (width_mask - xoffs);
			}

			if (is_flipy)
			{
			    yoffs = (height_mask - yoffs);
			}

			sprite_code += xoffset.at(xoffs);
			sprite_code += yoffset.at(yoffs);

			drawNormalSprite(sprite_code, color_attrib, priority, is_shadow, xcoord, ycoord, is_flipx, is_flipy);
		    }
		}
	    }
	}
    }

    void k051960video::drawNormalSprite(uint32_t sprite_num, uint8_t pal_num, uint8_t priority, bool shadow, int xcoord, int ycoord, bool flipx, bool flipy)
    {
	uint32_t sprite_offs = (sprite_num * 256);

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

	    if (!inRange(xpos, 0, 512) || !inRange(ypos, 0, 256))
	    {
		continue;
	    }

	    uint32_t color_num = (obj_tiles.at(sprite_offs + pixel) & 0xF);

	    if (color_num == 0)
	    {
		continue;
	    }

	    size_t pixel_offs = (xpos + (ypos * 512));

	    bool is_shadow = false;

	    if (color_num == 15)
	    {
		is_shadow = (shadow && !testbit(shadow_config, 0));
	    }
	    else
	    {
		is_shadow = testbit(shadow_config, 0);
	    }

	    auto &obj_pixel = obj_buffer.at(pixel_offs);
	    obj_pixel = (color_num | (pal_num << 4) | (priority << 12) | (is_shadow << 20));
	}
    }

    objbuffer k051960video::getFramebuffer()
    {
	return obj_buffer;
    }

    uint8_t k051960video::fetchROMData(int offs)
    {
	uint32_t addr = rom_offset;
	addr += ((sprite_rom_bank[0] << 8) + ((sprite_rom_bank[1] & 0x3) << 16));
	uint16_t sprite_code = ((addr & 0x3FFE0) >> 5);
	int off1 = (addr & 0x1F);
	uint8_t color = (((sprite_rom_bank[1] & 0xFC) << 2) + ((sprite_rom_bank[2] & 0x3) << 6));
	uint8_t priority = 0;
	bool is_shadow = false;

	if (spritecb)
	{
	    spritecb(sprite_code, color, priority, is_shadow);
	}

	addr = ((sprite_code << 7) | (off1 << 2) | (offs & 3));
	addr &= (obj_rom.size() - 1);

	return obj_rom.at(addr);
    }

    uint8_t k051960video::read(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr <= 7)
	{
	    switch (addr)
	    {
		case 4:
		case 5:
		case 6:
		case 7:
		{
		    data = fetchROMData((addr & 3));
		}
		break;
		default: cout << "Reading value from K051960 address of " << hex << int(addr) << endl; break;
	    }
	}
	else if (inRange(addr, 0x400, 0x800))
	{
	    if (is_rmrd)
	    {
		rom_offset = ((addr & 0x3FC) >> 2);
		data = fetchROMData(addr & 3);
	    }
	    else
	    {
		data = obj_ram.at((addr & 0x3FF));
	    }
	}

	return data;
    }

    void k051960video::write(uint16_t addr, uint8_t data)
    {
	if (addr <= 7)
	{
	    switch (addr)
	    {
		case 0:
		{
		    cout << "Writing value of " << hex << int(data) << " to K051960 address of 0" << endl;
		    is_rmrd = testbit(data, 5);
		}
		break;
		case 1:
		{
		    shadow_config = (data & 0x7);
		}
		break;
		case 2:
		case 3:
		case 4:
		{
		    sprite_rom_bank.at(addr - 2) = data;
		}
		break;
		default: cout << "Writing value of " << hex << int(data) << " to K051960 address of " << hex << int(addr) << endl; break;
	    }
	}
	else if (inRange(addr, 0x400, 0x800))
	{
	    obj_ram.at((addr & 0x3FF)) = data;
	}
    }
};