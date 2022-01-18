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

#ifndef BERRN_GALAGA_VIDEO_H
#define BERRN_GALAGA_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class galagavideo
    {
	public:
	    galagavideo(berrndriver &drv);
	    ~galagavideo();

	    bool init();
	    void shutdown();
	    void updatePixels();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    void writeSprites(int bank, uint16_t addr, uint8_t data);

	    int64_t timeUntilScanline(int scanline);

	private:
	    berrndriver &driver;

	    BerrnBitmapRGB *bitmap = NULL;

	    array<uint8_t, 0x400> vram;
	    array<uint8_t, 0x400> cram;

	    array<uint8_t, 0x80> obj_ram;
	    array<uint8_t, 0x80> obj_ram2;
	    array<uint8_t, 0x80> obj_ram3;

	    vector<uint8_t> tile_pal_rom;
	    vector<uint8_t> sprite_pal_rom;
	    
	    vector<uint8_t> col_rom;
	    vector<uint8_t> tile_rom;
	    vector<uint8_t> sprite_rom;

	    vector<uint8_t> tile_ram;
	    vector<uint8_t> sprite_ram;

	    void update_tiles();
	    void update_sprites();

	    void draw_tile(int tile_num, int pal_num, int xcoord, int ycoord);
	    void draw_sprite(int sprite_num, int pal_num, int xcoord, int ycoord, bool flipx, bool flipy);

	    void set_pixel(int xpos, int ypos, uint8_t color_num);
    };
};

#endif // BERRN_GALAGA_VIDEO_H