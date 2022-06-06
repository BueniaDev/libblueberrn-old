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

#ifndef BERRN_TMNT_VIDEO_H
#define BERRN_TMNT_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
#include <video/k052109.h>
#include <video/k051960.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class tmntvideo
    {
	public:
	    tmntvideo(berrndriver &drv);
	    ~tmntvideo();

	    void init();
	    void shutdown();

	    void setRMRD(bool line);
	    uint8_t tile_read(uint16_t addr);
	    void tile_write(uint16_t addr, uint8_t data);

	    uint8_t sprite_read(uint16_t addr);
	    void sprite_write(uint16_t addr, uint8_t data);

	    uint8_t palette_read(uint32_t addr);
	    void palette_write(uint32_t addr, uint8_t data);

	    void set_priority(int data);

	    void updatePixels();

	    void spritedump();

	private:
	    berrndriver &driver;
	    k052109video *tilemap = NULL;
	    k051960video *spritemap = NULL;

	    void updateSprites();

	    bool dump = false;
	    int frame_number = 0;
	    int priority_flag = 0;

	    vector<uint8_t> obj_tiles;

	    BerrnBitmapRGB *bitmap = NULL;

	    void chunky_to_planar(vector<uint8_t> &rom);

	    void render_tile_layer(int layer, bool is_opaque);

	    BerrnPaletteXBGR555 *palette = NULL;
    };
};

#endif // BERRN_TMNT_VIDEO_H