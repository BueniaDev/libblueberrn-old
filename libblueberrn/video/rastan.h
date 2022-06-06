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

#ifndef BERRN_RASTAN_VIDEO
#define BERRN_RASTAN_VIDEO

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
#include "pc080sn.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    class rastanvideo
    {
	public:
	    rastanvideo(berrndriver &drv);
	    ~rastanvideo();

	    void init();
	    void shutdown();

	    void updatePixels();

	    uint16_t readPC080SN(int bank, bool upper, bool lower, uint32_t addr);
	    void writePC080SN(int bank, bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;
	    pc080snvideo *tilemap = NULL;

	    BerrnBitmapRGB *bitmap = NULL;

	    vector<uint8_t> bg_tiles;

	    void renderBG(uint32_t tile_num, int xcoord, int ycoord);
    };
};

#endif // BERRN_RASTAN_VIDEO