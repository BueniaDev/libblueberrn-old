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

	    bool init();
	    void shutdown();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    uint8_t readSprites(uint16_t addr);
	    void writeSprites(uint16_t addr, uint8_t data);

	    void setScroll(bool is_msb, uint8_t data);
	    void setPaletteBank(uint8_t data);

	    void update_pixels();

	private:
	    berrndriver &driver;
	    BerrnBitmapRGB *bitmap = NULL;

	    array<uint8_t, 0x800> fg_vram;
	    array<uint8_t, 0x400> bg_vram;
	    array<uint8_t, 0x80> sprite_ram;

	    void init_palette();

	    int palette_bank = 0;

	    vector<uint8_t> bg_rom;
	    vector<uint8_t> fg_rom;

	    array<uint8_t, 256> fg_pal;
	    array<array<uint8_t, 256>, 4> bg_pal;

	    array<berrnRGBA, 256> colors;

	    vector<uint8_t> bg_ram;
	    vector<uint8_t> fg_ram;

	    void update_background();
	    void update_foreground();

	    uint16_t scroll_x = 0;

	    void draw_bg_tile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord, bool xflip, bool yflip);
	    void draw_fg_tile(uint32_t tile_num, uint32_t pal_num, int xcoord, int ycoord);
    };
};

#endif // BERRN_1942_VIDEO_H