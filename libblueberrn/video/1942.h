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
    class berrn1942video
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

	    void writeScroll(bool is_msb, uint8_t data);
	    void writePaletteBank(uint8_t data);

	private:
	    berrndriver &driver;
	    array<uint8_t, 0x400> bg_vram;
	    array<uint8_t, 0x800> fg_vram;
	    array<uint8_t, 0x80> obj_ram;

	    void updateBG();
	    void updateSprites();
	    void updateFG();

	    void renderSprite(int sprite_num, int color, int xcoord, int ycoord);

	    void initPalettes();

	    vector<uint8_t> bg_tiles;
	    vector<uint8_t> fg_tiles;
	    vector<uint8_t> obj_tiles;

	    array<berrnRGBA, 256> colors;
	    array<array<uint8_t, 0x100>, 4> bg_palettes;
	    array<uint8_t, 0x100> fg_palettes;
	    array<uint8_t, 0x100> obj_palettes;

	    uint16_t scrollx = 0;
	    int palette_bank = 0;

	    int clip_miny = 0;
	    int clip_maxy = 0;

	    BerrnBitmapRGB *bitmap = NULL;
    };
};

#endif // BERRN_1942_VIDEO_H