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

#include "mia.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout mia_obj_layout = 
    {
	16, 16,
	8192,
	4,
	{24, 16, 8, 0},
	{gfx_step8(0, 1), gfx_step8(256, 1)},
	{gfx_step8(0, 32), gfx_step8(512, 32)},
	128*8
    };

    miavideo::miavideo(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(304, 224);
	bitmap->clear();

	tilemap = new k052109video(driver);
	spritemap = new k051960video(driver);
	palette = new BerrnPaletteXBGR555(1024, true);
    }

    miavideo::~miavideo()
    {

    }

    void miavideo::init()
    {
	auto tile_callback = [&](int layer, uint8_t tile_num, uint8_t color_attrib, int bank) -> uint32_t
	{
	    uint32_t tile_addr = (tile_num | ((color_attrib & 0x1) << 8));

	    if (layer != 0)
	    {
		tile_addr |= (((color_attrib & 0x18) << 6) | (bank << 11));
	    }

	    bool is_flipx = testbit(color_attrib, 2);

	    uint8_t attrib_byte = color_attrib;

	    if (layer == 1)
	    {
		attrib_byte = changebit(attrib_byte, 2, is_flipx);
	    }
	    else
	    {
		attrib_byte = changebit(attrib_byte, 0, is_flipx);
	    }

	    return tilemap->create_tilemap_addr(tile_addr, attrib_byte);
	};

	auto sprite_callback = [&](uint16_t&, uint8_t &color_attrib, uint8_t&, bool&) -> void
	{
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

	for (size_t index = 0; index < sprite_temp.size(); index++)
	{
	    int temp_value = (index & 0x3FF00);

	    if ((index & 0x3C000) == 0x3C000)
	    {
		temp_value |= bitswap<8>(index, 7, 6, 4, 2, 1, 0, 5, 3);
	    }
	    else
	    {
		temp_value |= bitswap<8>(index, 6, 4, 2, 1, 0, 7, 5, 3);
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

	gfxDecodeSet(mia_obj_layout, sprite_rom, obj_tiles);

	tilemap->setCallback(tile_callback);
	tilemap->init();
	tilemap->setROM(tile_rom);

	spritemap->init();
	spritemap->setROM(sprite_rom);
	spritemap->setTiles(obj_tiles);
	spritemap->setSpriteCallback(sprite_callback);
    }

    void miavideo::shutdown()
    {
	tilemap->shutdown();
	spritemap->shutdown();
	bitmap->clear();
	palette->clear();
    }

    void miavideo::chunky_to_planar(vector<uint8_t> &rom)
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

    void miavideo::updatePixels()
    {
	render_tile_layer(2, true);
	render_tile_layer(1, false);
	updateSprites();
	render_tile_layer(0, false);
	driver.set_screen(bitmap);
    }

    void miavideo::updateSprites()
    {
	spritemap->render(0, 0);
	auto obj_buffer = spritemap->getFramebuffer();

	for (int xpos = 104; xpos < 408; xpos++)
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
		bitmap->setPixel((xpos - 104), (ypos - 16), palette->getColor(palette_num));
	    }
	}
    }

    void miavideo::render_tile_layer(int layer, bool is_opaque)
    {
	array<int, 3> layer_colorbase = {0, 32, 40};
	auto gfx_addr = tilemap->render(layer);

	for (int xpos = 104; xpos < 408; xpos++)
	{
	    for (int ypos = 16; ypos < 240; ypos++)
	    {
		uint32_t pixel_offs = (xpos + (ypos * 512));
		uint32_t tile_addr = (gfx_addr.at(pixel_offs) & 0xFF);
		int color_num = layer_colorbase.at(layer);

		if (layer == 0)
		{
		    color_num += (((tile_addr & 0x80) >> 5) + ((tile_addr & 0x10) >> 1));
		}
		else
		{
		    color_num += ((tile_addr & 0xE0) >> 5);
		}

		int tile_num = (tile_addr & 0xF);

		if (!is_opaque && (tile_num == 0))
		{
		    continue;
		}

		int palette_num = ((color_num * 16) + tile_num);

		bitmap->setPixel((xpos - 104), (ypos - 16), palette->getColor(palette_num));
	    }
	}
    }

    uint16_t miavideo::tile_read(bool upper, bool lower, uint32_t addr)
    {
	return tilemap->read16(upper, lower, addr);
    }

    void miavideo::tile_write(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	tilemap->write16(upper, lower, addr, data);
    }

    uint8_t miavideo::sprite_read(uint16_t addr)
    {
	return spritemap->read(addr);
    }

    void miavideo::sprite_write(uint16_t addr, uint8_t data)
    {
	spritemap->write(addr, data);
    }

    uint8_t miavideo::palette_read(uint32_t addr)
    {
	addr = ((addr >> 1) & 0x7FF);
	return palette->read8(addr);
    }

    void miavideo::palette_write(uint32_t addr, uint8_t data)
    {
	addr = ((addr >> 1) & 0x7FF);
	palette->write8(addr, data);
    }

    void miavideo::setRMRD(bool line)
    {
	tilemap->setRMRD(line);
    }
};

