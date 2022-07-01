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

#ifndef BERRN_1943_VIDEO_H
#define BERRN_1943_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class berrn1943video
    {
	public:
	    berrn1943video(berrndriver &drv);
	    ~berrn1943video();

	    void writeBG2Scroll(bool is_msb, uint8_t data);
	    void writeBGScrollX(bool is_msb, uint8_t data);
	    void writeBGScrollY(uint8_t data);
	    void writeD806(uint8_t data);

	    void init();
	    void shutdown();
	    void updatePixels();

	private:
	    berrndriver &driver;

	    void updateBG1();
	    void updateBG2();

	    void initPalettes();

	    BerrnBitmapRGB *bitmap = NULL;

	    vector<uint8_t> bg_tiles;
	    vector<uint8_t> bg2_tiles;

	    void setPriorPixel(int xpos, int ypos, int bit)
	    {
		if (!inRange(xpos, 0, 256) || !inRange(ypos, 0, 224))
		{
		    return;
		}

		int offs = (xpos + (ypos * 256));
		prior_bmp.at(offs) = setbit(prior_bmp.at(offs), bit);
	    }

	    vector<uint8_t> tile_rom;

	    array<uint8_t, 0x100> bg_palettes;
	    array<uint8_t, 0x100> fg_palettes;
	    array<berrnRGBA, 256> colors;

	    array<uint8_t, (256 * 224)> prior_bmp;

	    array<bool, 256> is_fg_transparent;

	    bool is_bg1_enabled = false;
	    bool is_bg2_enabled = false;

	    uint16_t bg2_scrollx = 0;
	    uint16_t bg_scrollx = 0;
	    uint8_t bg_scrolly = 0;
    };
};

#endif // BERRN_1943_VIDEO_H