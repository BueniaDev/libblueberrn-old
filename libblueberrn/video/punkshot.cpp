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

#include "punkshot.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout punkshot_obj_layout = 
    {
	16, 16,
	16384,
	4,
	{0, 8, 16, 24},
	{gfx_step8(0, 1), gfx_step8(256, 1)},
	{gfx_step8(0, 32), gfx_step8(512, 32)},
	128*8
    };

    punkshotvideo::punkshotvideo(berrndriver &drv) : driver(drv)
    {
	tilemap = new k052109video(driver);
	spritemap = new k051960video(driver);
	priormap = new k053251video(driver);

	bitmap = new BerrnBitmapRGB(288, 224);
	bitmap->clear();
    }

    punkshotvideo::~punkshotvideo()
    {

    }

    void punkshotvideo::init()
    {
	auto tile_callback = [&](int, uint8_t tile_num, uint8_t color_attrib, int bank) -> uint32_t
	{
	    uint32_t tile_addr = tile_num;
	    tile_addr |= (((color_attrib & 0x3) << 8) | ((color_attrib & 0x10) << 6) | ((color_attrib & 0xC) << 9) | (bank << 13));

	    uint8_t attrib_byte = (color_attrib & 0xE0);
	    return tilemap->create_tilemap_addr(tile_addr, attrib_byte);
	};

	auto sprite_callback = [&](uint16_t &code, uint16_t &color_attrib, uint8_t &priority, bool&) -> void
	{
	    priority = (0x20 | ((color_attrib & 0x60) >> 2));
	    code |= ((color_attrib & 0x10) << 9);
	    color_attrib = (color_attrib & 0xF);
	};

	auto tile_rom = driver.get_rom_region("k052109");
	auto sprite_rom = driver.get_rom_region("k051960");

	// gfxDecodeSet(punkshot_obj_layout, sprite_rom, obj_tiles);

	tilemap->setCallback(tile_callback);
	tilemap->init();
	tilemap->setROM(tile_rom);

	spritemap->init();
	spritemap->setROM(sprite_rom);
	// spritemap->setTiles(obj_tiles);
	spritemap->setSpriteCallback(sprite_callback);
	priormap->init();
    }

    void punkshotvideo::shutdown()
    {
	tilemap->shutdown();
	spritemap->shutdown();
	bitmap->clear();
    }

    bool punkshotvideo::isIRQEnabled()
    {
	return tilemap->isIRQEnabled();
    }

    void punkshotvideo::updatePixels()
    {
	// TODO: Finish implementing video logic
	layer0 = tilemap->render(0);
	layer1 = tilemap->render(1);
	layer2 = tilemap->render(2);

	spritemap->render(0, 0);
	objlayer = spritemap->getFramebuffer();

	priormap->setPriority(0, 0x3F);
	priormap->setPriority(2, 0x3F);

	priormap->setInput(0, 0);

	for (int xpos = 0; xpos < 288; xpos++)
	{
	    for (int ypos = 0; ypos < 224; ypos++)
	    {
		int bg0_offs = ((112 + xpos) + ((ypos + 16) * 512));
		int bg1_offs = ((106 + xpos) + ((ypos + 16) * 512));

		int bg0 = layer0.at(bg0_offs);
		int bg1 = layer1.at(bg1_offs);
		int bg2 = layer2.at(bg1_offs);
		int obj = objlayer.at(bg0_offs);

		int bg0_tilenum = (bg0 & 0xF);
		int bg0_color = ((bg0 >> 5) & 0x7);
		int bg0_input = ((bg0_color << 4) | bg0_tilenum);

		int bg1_tilenum = (bg1 & 0xF);
		int bg1_color = ((bg1 >> 5) & 0x7);
		int bg1_input = ((bg1_color << 4) | bg1_tilenum);

		int bg2_tilenum = (bg2 & 0xF);
		int bg2_color = ((bg2 >> 5) & 0x7);
		int bg2_input = ((bg2_color << 4) | bg2_tilenum);

		int obj_tilenum = (obj & 0xF);
		int obj_color = ((obj >> 4) & 0xFF);
		int obj_prior = ((obj >> 12) & 0xFF);
		bool obj_shadow = testbit(obj, 20);
		int obj_input = ((obj_color << 4) | obj_tilenum);

		int shadow = (obj_shadow) ? 1 : 0;

		priormap->setInput(1, 0);
		priormap->setInput(2, bg0_input);
		priormap->setInput(3, bg2_input);
		priormap->setInput(4, bg1_input);

		berrnRGBA prev_color = getColor();

		priormap->setPriority(1, obj_prior);
		// priormap->setShadow(shadow);
		priormap->setInput(1, obj_input);

		berrnRGBA color = getColor();

		bool is_shadow = obj_shadow;
		// bool is_shadow = testbit(priormap->getShadow(), 0);

		if (is_shadow)
		{
		    color = (prev_color * 0.60);
		}

		bitmap->setPixel(xpos, ypos, color);
	    }
	}


	driver.set_screen_bmp(bitmap);
    }

    void punkshotvideo::setRMRD(bool line)
    {
	tilemap->setRMRD(line);
    }

    uint16_t punkshotvideo::tileRead(bool upper, bool lower, uint32_t addr)
    {
	return tilemap->read16(upper, lower, addr);
    }

    void punkshotvideo::tileWrite(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	tilemap->write16(upper, lower, addr, data);
    }

    uint8_t punkshotvideo::spriteRead(uint16_t addr)
    {
	return spritemap->read(addr);
    }

    void punkshotvideo::spriteWrite(uint16_t addr, uint8_t data)
    {
	spritemap->write(addr, data);
    }

    uint16_t punkshotvideo::paletteRead(bool upper, bool lower, uint32_t addr)
    {
	addr &= 0xFFF;
	uint16_t data = 0;

	if (upper)
	{
	    data |= (palette_ram.at(addr) << 8);
	}

	if (lower)
	{
	    data |= palette_ram.at(addr + 1);
	}

	return data;
    }

    void punkshotvideo::paletteWrite(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	addr &= 0xFFF;
	
	if (upper)
	{
	    palette_ram.at(addr) = (data >> 8);
	}

	if (lower)
	{
	    palette_ram.at(addr + 1) = (data & 0xFF);
	}
    }

    void punkshotvideo::priorityWrite(uint32_t addr, uint8_t data)
    {
	int reg = ((addr >> 1) & 0xF);
	priormap->write(reg, data);
    }

    berrnRGBA punkshotvideo::getColor()
    {
	auto layer = priormap->getLayer();
	uint16_t output = priormap->getOutput(layer);
	int palette_index = priormap->getPaletteIndex(layer);

	int color_num = (palette_index | (output >> 4));
	int tile_num = (output & 0xF);

	int palette_num = ((color_num * 16) + tile_num);

	int palette_offs = (palette_num * 2);

	uint8_t high = palette_ram.at(palette_offs);
	uint8_t low = palette_ram.at(palette_offs + 1);
	uint16_t color = ((high << 8) | low);

	int red_val = (color & 0x1F);
	int green_val = ((color >> 5) & 0x1F);
	int blue_val = ((color >> 10) & 0x1F);

	int red = ((red_val << 3) | (red_val >> 2));
	int green = ((green_val << 3) | (green_val >> 2));
	int blue = ((blue_val << 3) | (blue_val >> 2));

	return fromRGB(red, green, blue);
    }
};