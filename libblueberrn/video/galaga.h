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

#ifndef BERRN_GALAGA_VIDEO_H
#define BERRN_GALAGA_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class galagavideo
    {
	public:
	    galagavideo(berrndriver &drv);
	    ~galagavideo();

	    void init();
	    void shutdown();
	    void updatePixels();

	    uint8_t readVRAM(uint16_t addr);
	    void writeVRAM(uint16_t addr, uint8_t data);

	    uint8_t readORAM(int bank, uint16_t addr);
	    void writeORAM(int bank, uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    array<uint8_t, 0x400> video_ram;
	    array<uint8_t, 0x400> color_ram;
	    array<array<uint8_t, 0x80>, 3> sprite_ram;

	    vector<uint8_t> tile_pal_rom;
	    vector<uint8_t> color_rom;

	    vector<uint8_t> tile_ram;

	    uint32_t tilemap_scan(uint32_t row, uint32_t col);
	    void draw_tile(uint32_t tile_num, int xcoord, int ycoord, int pal_num);

	    void set_pixel(int xpos, int ypos, uint8_t color_num);

	    BerrnBitmapRGB *bitmap = NULL;
    };
};

#endif // BERRN_GALAGA_VIDEO_H