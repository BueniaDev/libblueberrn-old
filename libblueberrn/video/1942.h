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

#ifndef BERRN_1942_VIDEO_H
#define BERRN_1942_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API berrn1942video
    {
	public:
	    berrn1942video(berrndriver &drv);
	    ~berrn1942video();

	    void init();
	    void shutdown();

	    void updatePixels();

	    uint8_t readBG(uint16_t addr);
	    void writeBG(uint16_t addr, uint8_t data);

	    uint8_t readFG(uint16_t addr);
	    void writeFG(uint16_t addr, uint8_t data);

	    uint8_t readOBJ(uint16_t addr);
	    void writeOBJ(uint16_t addr, uint8_t data);

	    void setScroll(bool is_msb, uint8_t data);
	    void setPaletteBank(uint8_t data);

	private:
	    berrndriver &driver;

	    array<uint8_t, 0x80> obj_ram;
	    array<uint8_t, 0x800> fg_ram;
	    array<uint8_t, 0x400> bg_ram;

	    BerrnBitmapRGB *bitmap = NULL;

	    void init_palette();

	    void updateBackground();
	    void updateSprites();
	    void updateForeground();

	    void drawBGTile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord, bool flipx, bool flipy);
	    void drawFGTile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord);
	    void drawSprite(uint32_t sprite_num, uint32_t pal_num, int xcoord, int ycoord);

	    uint16_t scroll_x = 0;
	    int palette_bank = 0;

	    vector<uint8_t> bg_tiles;

	    vector<uint8_t> fg_tiles;

	    vector<uint8_t> obj_tiles;

	    array<array<uint8_t, 256>, 4> bg_pal;
	    array<uint8_t, 256> fg_pal;
	    array<uint8_t, 256> obj_pal;

	    array<berrnRGBA, 256> colors;

	    int clip_min = 0;
	    int clip_max = 0;
    };
};

#endif // BERRN_1942_VIDEO_H