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

#ifndef BERRN_K051960_VIDEO_H
#define BERRN_K051960_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    using objbuffer = array<int, (512 * 256)>;
    using k051960callback = function<void(uint16_t&, uint8_t&, uint8_t&, bool&)>;
    class k051960video
    {
	public:
	    k051960video(berrndriver &drv);
	    ~k051960video();

	    void init();
	    void setROM(vector<uint8_t> &rom);
	    void setTiles(vector<uint8_t> &tiles);
	    void setSpriteCallback(k051960callback cb);
	    void shutdown();

	    void render(int min_priority, int max_priority);
	    objbuffer getFramebuffer();

	    uint8_t read(uint16_t addr);
	    void write(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    vector<uint8_t> obj_rom;
	    vector<uint8_t> obj_tiles;

	    array<uint8_t, 0x400> obj_ram;

	    bool is_rmrd = false;

	    array<uint8_t, 4> sprite_rom_bank;

	    uint8_t fetchROMData(int offs);

	    uint32_t rom_offset = 0;

	    void drawNormalSprite(uint32_t sprite_num, uint8_t pal_num, uint8_t priority, bool shadow, int xcoord, int ycoord, bool flipx, bool flipy);

	    objbuffer obj_buffer;

	    k051960callback spritecb;

	    int shadow_config = 0;
    };
};

#endif // BERRN_K051960MAME_VIDEO_H