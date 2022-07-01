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

#include "rastan.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout bg_layout = 
    {
	8, 8,
	16384,
	4,
	{ gfx_step4(0, 1) },
	{ gfx_step8(0, 4) },
	{ gfx_step8(0, 32) },
	256
    };

    rastanvideo::rastanvideo(berrndriver &drv) : driver(drv)
    {
	tilemap = new pc080snvideo(driver);
	bitmap = new BerrnBitmapRGB(320, 240);
	bitmap->clear();
    }

    rastanvideo::~rastanvideo()
    {

    }

    void rastanvideo::init()
    {
	tilemap->init();
	auto bg_rom = driver.get_rom_region("pc080sn");
	gfxDecodeSet(bg_layout, bg_rom, bg_tiles);
    }

    void rastanvideo::shutdown()
    {
	bg_tiles.clear();
	tilemap->shutdown();
	bitmap->clear();
    }

    void rastanvideo::updatePixels()
    {
	tilemap->render();

	bitmap->fillcolor(black());
	auto bg_framebuffer = tilemap->getFramebuffer(0);

	for (int row = 1; row < 31; row++)
	{
	    for (int col = 0; col < 40; col++)
	    {
		size_t offs = ((row * 64) + col);
		int xpos = (col * 8);
		int ypos = ((row * 8) - 8);

		uint32_t bg_index = bg_framebuffer.at(offs);
		uint32_t tile_number = (bg_index & 0x3FFF);

		if (tile_number == 0)
		{
		    continue;
		}

		renderBG(tile_number, xpos, ypos);
	    }
	}

	driver.set_screen_bmp(bitmap);
    }

    void rastanvideo::renderBG(uint32_t tile_num, int xcoord, int ycoord)
    {
	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < 64; pixel++)
	{
	    int py = (pixel / 8);
	    int px = (pixel % 8);

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    uint32_t tile_color = bg_tiles.at((tile_num * 64) + pixel);

	    bitmap->setPixel(xpos, ypos, fromLevel(tile_color, 4));
	}
    }

    uint16_t rastanvideo::readPC080SN(int bank, bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;

	switch (bank)
	{
	    case 0: data = tilemap->readRAM(upper, lower, addr); break;
	}

	return data;
    }

    void rastanvideo::writePC080SN(int bank, bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	switch (bank)
	{
	    case 0: tilemap->writeRAM(upper, lower, addr, data); break;
	    case 1: tilemap->writeScrollY(upper, lower, addr, data); break;
	    case 2: tilemap->writeScrollX(upper, lower, addr, data); break;
	    case 3: tilemap->writeControl(upper, lower, addr, data); break;
	}
    }
};