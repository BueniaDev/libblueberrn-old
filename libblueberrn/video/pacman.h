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

#ifndef BERRN_PACMAN_VIDEO_H
#define BERRN_PACMAN_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class pacmanvideo
    {
	public:
	    pacmanvideo(berrndriver &drv);
	    ~pacmanvideo();

	    void init();
	    void shutdown();
	    void updatePixels();

	    uint8_t readVRAM(uint16_t addr);
	    void writeVRAM(uint16_t addr, uint8_t data);

	    uint8_t readORAM(uint16_t addr);
	    void writeORAM(int bank, uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    array<uint8_t, 0x400> video_ram;
	    array<uint8_t, 0x400> color_ram;
	    array<array<uint8_t, 0x10>, 2> obj_ram;

	    vector<uint8_t> pal_rom;
	    vector<uint8_t> color_rom;

	    vector<uint8_t> tile_ram;
	    vector<uint8_t> sprite_ram;

	    void draw_tile(uint32_t tile_num, int xcoord, int ycoord, int pal_num);
	    void draw_sprite(uint32_t sprite_num, int xcoord, int ycoord, int pal_num, bool flipx, bool flipy);
	    void set_pixel(int xpos, int ypos, uint8_t color_num);

	    BerrnBitmapRGB *bitmap = NULL;

	    uint32_t tilemap_scan(uint32_t row, uint32_t col);
    };
};

#endif // BERRN_PACMAN_VIDEO_H