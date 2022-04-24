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

#ifndef BERRN_GALAXIAN_VIDEO_H
#define BERRN_GALAXIAN_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API galaxianvideo
    {
	public:
	    galaxianvideo(berrndriver &drv);
	    ~galaxianvideo();

	    void init();
	    void shutdown();

	    void updatePixels();

	    uint8_t readVRAM(int addr);
	    void writeVRAM(int addr, uint8_t data);

	    uint8_t readORAM(int addr);
	    void writeORAM(int addr, uint8_t data);

	    void writeIO(int reg, bool line);

	private:
	    berrndriver &driver;
	    BerrnBitmapRGB *bitmap = NULL;

	    struct StarfieldItem
	    {
		int xpos = 0;
		int ypos = 0;
		int color = 0;
	    };

	    array<StarfieldItem, 2520> starfield;

	    void init_starfield();
	    int starfield_scroll_pos = 0;

	    void draw_starfield();
	    void draw_star_pixel(int xpos, int ypos, int color);

	    void update_star_scroll();

	    bool is_starfield_enabled = false;

	    void draw_tile(uint32_t tile_num, int xcoord, int ycoord, int pal_num, int scroll);
	    void set_pixel(int xpos, int ypos, uint8_t color);

	    array<uint8_t, 0x400> video_ram;
	    array<uint8_t, 0x100> obj_ram;

	    vector<uint8_t> pal_rom;
	    vector<uint8_t> tile_rom;

	    vector<uint8_t> tile_ram;
    };
};


#endif // BERRN_GALAXIAN_VIDEO_H