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
    class galaxianvideo
    {
	public:
	    galaxianvideo(berrndriver &drv);
	    ~galaxianvideo();

	    bool init();
	    void shutdown();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);
	    void update_latch(int addr, uint8_t data);
	    void update_pixels();

	    BerrnBitmapRGB *get_bitmap()
	    {
		return bitmap;
	    }

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

	    void update_star_scroll();
	    void draw_starfield();
	    void draw_star_pixel(int xpos, int ypos, int color);

	    vector<uint8_t> pal_rom;
	    vector<uint8_t> tile_rom;

	    array<uint8_t, 0x400> video_ram;
	    array<uint8_t, 0x100> obj_ram;

	    vector<uint8_t> tile_ram;
	    vector<uint8_t> sprite_ram;

	    bool is_stars_enabled = false;
	    bool is_flip_screenx = false;
	    bool is_flip_screeny = false;

	    void draw_tile(uint8_t tile_num, int x, int y, int pal_num);
	    void draw_sprite(uint8_t sprite_num, int x, int y, bool flip_x, bool flip_y, int pal_num);
	    void set_pixel(int xpos, int ypos, uint8_t color);
    };
};


#endif // BERRN_GALAXIAN_VIDEO_H