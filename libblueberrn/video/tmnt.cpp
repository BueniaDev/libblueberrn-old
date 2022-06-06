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

#include "tmnt.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout tmnt_obj_layout = 
    {
	16, 16,
	16384,
	4,
	{24, 16, 8, 0},
	{gfx_step8(0, 1), gfx_step8(256, 1)},
	{gfx_step8(0, 32), gfx_step8(512, 32)},
	128*8
    };

    tmntvideo::tmntvideo(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(320, 224);
	bitmap->clear();

	palette = new BerrnPaletteXBGR555(1024, true);

	tilemap = new k052109video(driver);
	spritemap = new k051960video(driver);
    }

    tmntvideo::~tmntvideo()
    {

    }

    void tmntvideo::init()
    {
	auto tile_callback = [&](uint8_t tile_num, uint8_t color_attrib, int bank) -> uint32_t
	{
	    uint32_t tile_addr = tile_num;
	    tile_addr |= (((color_attrib & 0x3) << 8) | ((color_attrib & 0x10) << 6) | ((color_attrib & 0xC) << 9) | (bank << 13));

	    return tile_addr;
	};

	auto sprite_callback = [&](uint16_t &code, uint8_t &color_attrib, int &priority, int &shadow) -> void
	{
	    code |= ((color_attrib & 0x10) << 9);
	    color_attrib = (16 + (color_attrib & 0xF));
	};

	auto tile_rom = driver.get_rom_region("k052109");
	chunky_to_planar(tile_rom);

	auto sprite_rom = driver.get_rom_region("k051960");
	chunky_to_planar(sprite_rom);

	vector<uint32_t> sprite_temp;
	vector<uint32_t> sprite_rom_u32;

	for (size_t addr = 0; addr < sprite_rom.size(); addr += 4)
	{
	    uint32_t data = ((sprite_rom[addr + 3] << 24) | (sprite_rom[addr + 2] << 16) | (sprite_rom[addr + 1] << 8) | sprite_rom[addr]);
	    sprite_temp.push_back(data);
	}

	sprite_rom_u32.resize(sprite_temp.size(), 0);

	auto code_conv_table = driver.get_rom_region("sprprom");

	array<array<uint8_t, 8>, 10> bit_pick_table = 
	{
	    3, 3, 3, 3, 3, 3, 3, 3,
	    0, 0, 5, 5, 5, 5, 5, 5,
	    1, 1, 0, 0, 0, 7, 7, 7,
	    2, 2, 1, 1, 1, 0, 0, 9,
	    4, 4, 2, 2, 2, 1, 1, 0,
	    5, 6, 4, 4, 4, 2, 2, 1,
	    6, 5, 6, 6, 6, 4, 4, 2,
	    7, 7, 7, 7, 8, 6, 6, 4,
	    8, 8, 8, 8, 7, 8, 8, 6,
	    9, 9, 9, 9, 9, 9, 9, 8
	};

	for (size_t index = 0; index < sprite_temp.size(); index++)
	{
	    uint32_t entry_addr = ((index & 0x7F800) >> 11);
	    int entry = (code_conv_table.at(entry_addr) & 0x7);

	    array<int, 32> bits;

	    for (int i = 0; i < 10; i++)
	    {
		bits[i] = testbit(index, i);
	    }

	    int temp_value = (index & 0x7FC00);

	    for (int i = 0; i < 10; i++)
	    {
		int bit_entry = bit_pick_table.at(i).at(entry);
		temp_value |= (bits.at(bit_entry) << i);
	    }

	    sprite_rom_u32.at(index) = sprite_temp.at(temp_value);
	}

	for (size_t addr = 0; addr < sprite_rom.size(); addr += 4)
	{
	    uint32_t rom_data = sprite_rom_u32.at((addr >> 2));

	    sprite_rom.at(addr) = uint8_t(rom_data);
	    sprite_rom[addr + 1] = uint8_t(rom_data >> 8);
	    sprite_rom[addr + 2] = uint8_t(rom_data >> 16);
	    sprite_rom[addr + 3] = uint8_t(rom_data >> 24);
	}

	gfxDecodeSet(tmnt_obj_layout, sprite_rom, obj_tiles);

	tilemap->setCallback(tile_callback);
	tilemap->init();
	tilemap->setROM(tile_rom);

	spritemap->init();
	spritemap->setTiles(obj_tiles);
	spritemap->setSpriteCallback(sprite_callback);
    }

    void tmntvideo::shutdown()
    {
	obj_tiles.clear();
	tilemap->shutdown();
	spritemap->shutdown();
	bitmap->clear();
	palette->clear();
    }

    void tmntvideo::chunky_to_planar(vector<uint8_t> &rom)
    {
	for (size_t addr = 0; addr < rom.size(); addr += 4)
	{
	    uint32_t data = ((rom[addr + 3] << 24) | (rom[addr + 2] << 16) | (rom[addr + 1] << 8) | rom[addr]);

	    data = bitswap<32>(data, 
		31, 27, 23, 19, 15, 11, 7, 3, 
		30, 26, 22, 18, 14, 10, 6, 2, 
		29, 25, 21, 17, 13, 9,  5, 1, 
		28, 24, 20, 16, 12, 8,  4, 0
	    );

	    rom[addr] = uint8_t(data);
	    rom[addr + 1] = uint8_t(data >> 8);
	    rom[addr + 2] = uint8_t(data >> 16);
	    rom[addr + 3] = uint8_t(data >> 24);
	}
    }

    void tmntvideo::updatePixels()
    {
	render_tile_layer(2, true);

	if (testbit(priority_flag, 0))
	{
	    updateSprites();
	}

	render_tile_layer(1, false);

	if (!testbit(priority_flag, 0))
	{
	    updateSprites();
	}

	render_tile_layer(0, false);
	driver.set_screen(bitmap);
    }

    void tmntvideo::updateSprites()
    {
	spritemap->render(0, 0);
	auto obj_buffer = spritemap->getFramebuffer();

	// NOTE: These measurements match recordings taken from a real PCB
	for (int xpos = 102; xpos < 422; xpos++)
	{
	    for (int ypos = 16; ypos < 240; ypos++)
	    {
		size_t index = (xpos + (ypos * 512));
		uint32_t sprite_buffer = obj_buffer.at(index);
		int color_num = (sprite_buffer & 0xF);
		int color_attrib = (sprite_buffer >> 4);

		if (color_num == 0)
		{
		    continue;
		}

		int palette_num = ((color_attrib * 16) + color_num);

		bitmap->setPixel((xpos - 102), (ypos - 16), palette->getColor(palette_num));
	    }
	}
    }

    void tmntvideo::spritedump()
    {
    }

    void tmntvideo::render_tile_layer(int layer, bool is_opaque)
    {
	array<int, 3> layer_colorbase = {0, 32, 40};
	auto gfx_addr = tilemap->render(layer);

	// NOTE: These measurements match recordings taken from a real PCB
	for (int xpos = 96; xpos < 416; xpos++)
	{
	    for (int ypos = 16; ypos < 240; ypos++)
	    {
		uint32_t pixel_offs = (xpos + (ypos * 512));
		uint32_t tile_addr = (gfx_addr.at(pixel_offs) & 0xFF);

		int tile_num = (tile_addr & 0xF);

		int color_base = layer_colorbase.at(layer);

		int color_num = (color_base + ((tile_addr >> 5) & 0x7));

		if (!is_opaque && (tile_num == 0))
		{
		    continue;
		}

		int palette_num = ((color_num * 16) + tile_num);
		bitmap->setPixel((xpos - 96), (ypos - 16), palette->getColor(palette_num));
	    }
	}
    }

    uint8_t tmntvideo::tile_read(uint16_t addr)
    {
	return tilemap->read(addr);
    }

    void tmntvideo::tile_write(uint16_t addr, uint8_t data)
    {
	tilemap->write(addr, data);
    }

    uint8_t tmntvideo::sprite_read(uint16_t addr)
    {
	return spritemap->read(addr);
    }

    void tmntvideo::sprite_write(uint16_t addr, uint8_t data)
    {
	spritemap->write(addr, data);
    }

    uint8_t tmntvideo::palette_read(uint32_t addr)
    {
	addr &= 0x7FF;
	return palette->read8(addr);
    }

    void tmntvideo::palette_write(uint32_t addr, uint8_t data)
    {
	addr &= 0x7FF;
	palette->write8(addr, data);
    }

    void tmntvideo::set_priority(int data)
    {
	priority_flag = (data & 0x3);
    }

    void tmntvideo::setRMRD(bool line)
    {
	tilemap->setRMRD(line);
    }
};