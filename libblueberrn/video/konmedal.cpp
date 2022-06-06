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

#include "konmedal.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    shuriboyvideo::shuriboyvideo(berrndriver &drv) : driver(drv)
    {
	bitmap = new BerrnBitmapRGB(288, 224);
	bitmap->clear();

	tilemap = new k052109video(driver);
    }

    shuriboyvideo::~shuriboyvideo()
    {

    }

    void shuriboyvideo::init()
    {
	auto tile_callback = [&](uint8_t tile_num, uint8_t color_attrib, int bank) -> uint32_t
	{
	    uint32_t tile_addr = tile_num;
	    tile_addr |= (((color_attrib & 0xC) << 6) | (bank << 10));

	    if (testbit(color_attrib, 1))
	    {
		tile_addr = setbit(tile_addr, 12);
	    }

	    return tile_addr;
	};

	initLayerOrder();

	auto pal_rom = driver.get_rom_region("proms");

	for (int i = 0; i < 256; i++)
	{
	    uint8_t red = (pal_rom[i] << 4);
	    uint8_t green = (pal_rom[0x100 + i] << 4);
	    uint8_t blue = (pal_rom[0x200 + i] << 4);

	    colors.at(i) = fromRGB(red, green, blue);
	}

	auto tile_rom = driver.get_rom_region("k052109");

	tilemap->setCallback(tile_callback);
	tilemap->init();
	tilemap->setROM(tile_rom);
	// tilemap->startLog("shuriboy_log.bin");
    }

    void shuriboyvideo::shutdown()
    {
	tilemap->shutdown();
	bitmap->clear();
    }

    void shuriboyvideo::dump()
    {
	tilemap->startLog("shuriboy_log.bin");
	frame_number = 0;
	is_dump_active = true;
    }

    void shuriboyvideo::initLayerOrder()
    {
	setLayerOrder(2, 1, 0);
    }

    void shuriboyvideo::updatePixels()
    {
	bitmap->fillcolor(black());
	for (int prior = 0; prior < 2; prior++)
	{
	    for (int layer = 0; layer < 3; layer++)
	    {
		render(layer_order.at(layer), prior);
	    }
	}

	tilemap->logRender();
	driver.set_screen(bitmap);

	if (is_dump_active)
	{
	    frame_number += 1;

	    if (frame_number == 1000)
	    {
		tilemap->stopLog();
		is_dump_active = false;
	    }
	}
    }

    void shuriboyvideo::render(int layer, int priority)
    {
	bool prior = (priority == 0) ? true : false;
	auto gfx_addr = tilemap->render(layer);

	for (int xpos = 112; xpos < 400; xpos++)
	{
	    for (int ypos = 16; ypos < 240; ypos++)
	    {
		uint32_t pixel_offs = (xpos + (ypos * 512));
		uint32_t tile_addr = (gfx_addr.at(pixel_offs) & 0xFF);

		int color_base = (layer == 0) ? 0 : 8;

		int color_num = (color_base + ((tile_addr >> 5) & 0x7));
		int tile_num = (tile_addr & 0xF);

		bool is_prior = testbit(tile_addr, 4);

		if (is_prior == prior)
		{
		    continue;
		}

		if (tile_num == 0)
		{
		    continue;
		}

		int palette_num = ((color_num * 16) + tile_num);
		bitmap->setPixel((xpos - 112), (ypos - 16), colors.at(palette_num));
	    }
	}
    }

    uint8_t shuriboyvideo::read(uint16_t addr)
    {
	return tilemap->read(addr);
    }

    void shuriboyvideo::write(uint16_t addr, uint8_t data)
    {
	tilemap->write(addr, data);
    }

    void shuriboyvideo::setRMRD(bool line)
    {
	tilemap->setRMRD(line);
    }

    mariorouvideo::mariorouvideo(berrndriver &drv) : shuriboyvideo(drv)
    {

    }

    mariorouvideo::~mariorouvideo()
    {

    }

    void mariorouvideo::initLayerOrder()
    {
	setLayerOrder(0, 1, 2);
    }
};