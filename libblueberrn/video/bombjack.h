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

#ifndef BERRN_BOMBJACK_VIDEO_H
#define BERRN_BOMBJACK_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API bombjackvideo
    {
	public:
	    bombjackvideo(berrndriver &drv);
	    ~bombjackvideo();

	    void init();
	    void shutdown();

	    void updatePixels();

	    uint8_t readVRAM(uint16_t addr);
	    void writeVRAM(uint16_t addr, uint8_t data);

	    uint8_t readCRAM(uint16_t addr);
	    void writeCRAM(uint16_t addr, uint8_t data);

	    void writeORAM(uint16_t addr, uint8_t data);

	    void writePalette(uint16_t addr, uint8_t data);

	    void writeBackground(uint8_t data);

	private:
	    berrndriver &driver;
	    BerrnBitmapRGB *bitmap = NULL;

	    BerrnPaletteXBGR444 *palette = NULL;

	    void updateBackground();
	    void updateForeground();
	    void updateSprites();

	    void drawBGTile(uint16_t tile_num, int pal_num, int xcoord, int ycoord, bool flipy);
	    void drawFGTile(uint16_t tile_num, int pal_num, int xcoord, int ycoord);
	    void drawNormalSprite(uint16_t sprite_num, int pal_num, int xcoord, int ycoord, bool flipx, bool flipy);
	    void drawLargeSprite(uint16_t sprite_num, int pal_num, int xcoord, int ycoord, bool flipx, bool flipy);

	    vector<uint8_t> bg_tilemap;
	    vector<uint8_t> bg_tiles;
	    vector<uint8_t> fg_tiles;
	    vector<uint8_t> obj_tiles;
	    vector<uint8_t> obj_tiles2;

	    array<uint8_t, 0x400> vram;
	    array<uint8_t, 0x400> cram;

	    array<uint8_t, 0x60> obj_ram;
	    array<uint8_t, 0x100> pal_ram;

	    int background_number = 0;
    };
};

#endif // BERRN_BOMBJACK_VIDEO_H