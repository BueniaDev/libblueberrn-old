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

#ifndef BERRN_MIA_VIDEO_H
#define BERRN_MIA_VIDEO_H

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
    class miavideo
    {
	public:
	    miavideo(berrndriver &drv);
	    ~miavideo();

	    void init();
	    void shutdown();

	    uint16_t tile_read(bool upper, bool lower, uint32_t addr);
	    void tile_write(bool upper, bool lower, uint32_t addr, uint16_t data);

	    uint8_t sprite_read(uint16_t addr);
	    void sprite_write(uint16_t addr, uint8_t data);

	    uint8_t palette_read(uint32_t addr);
	    void palette_write(uint32_t addr, uint8_t data);

	    void setRMRD(bool line);

	    void updatePixels();

	private:
	    berrndriver &driver;

	    k052109video *tilemap = NULL;
	    k051960video *spritemap = NULL;

	    vector<uint8_t> obj_tiles;

	    BerrnBitmapRGB *bitmap = NULL;

	    void updateSprites();

	    void chunky_to_planar(vector<uint8_t> &rom);

	    void render_tile_layer(int layer, bool is_opaque);

	    BerrnPaletteXBGR555 *palette = NULL;

	    array<uint8_t, 0x800> pal_ram;
    };
};

#endif // BERRN_MIA_VIDEO_H