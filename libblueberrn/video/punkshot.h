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

#ifndef BERRN_PUNKSHOT_VIDEO_H
#define BERRN_PUNKSHOT_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
#include <video/k052109.h>
#include <video/k051960.h>
#include <video/k053251.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class punkshotvideo
    {
	public:
	    punkshotvideo(berrndriver &drv);
	    ~punkshotvideo();

	    void init();
	    void shutdown();

	    void updatePixels();

	    void setRMRD(bool line);
	    uint16_t tileRead(bool upper, bool lower, uint32_t addr);
	    void tileWrite(bool upper, bool lower, uint32_t addr, uint16_t data);

	    bool isIRQEnabled();

	    uint8_t spriteRead(uint16_t addr);
	    void spriteWrite(uint16_t addr, uint8_t data);

	    uint16_t paletteRead(bool upper, bool lower, uint32_t addr);
	    void paletteWrite(bool upper, bool lower, uint32_t addr, uint16_t data);

	    void priorityWrite(uint32_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    k052109video *tilemap = NULL;
	    k051960video *spritemap = NULL;
	    k053251video *priormap = NULL;

	    array<uint8_t, 0x1000> palette_ram;

	    array<int, (512 * 256)> layer0;
	    array<int, (512 * 256)> layer1;
	    array<int, (512 * 256)> layer2;
	    array<int, (512 * 256)> objlayer;

	    vector<uint8_t> obj_tiles;

	    berrnRGBA getColor();

	    BerrnBitmapRGB *bitmap = NULL;
    };
};


#endif // BERRN_PUNKSHOT_VIDEO_H