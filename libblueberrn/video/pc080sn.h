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

#ifndef BERRN_PC080SN_H
#define BERRN_PC080SN_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    using pc080addr = array<uint32_t, (64 * 64)>;
    class pc080snvideo
    {
	public:
	    pc080snvideo(berrndriver &drv);
	    ~pc080snvideo();

	    void init();
	    void shutdown();

	    void render();

	    pc080addr getFramebuffer(int layer);

	    uint16_t readRAM(bool upper, bool lower, uint32_t addr);
	    void writeRAM(bool upper, bool lower, uint32_t addr, uint16_t data);
	    void writeScrollX(bool upper, bool lower, uint32_t addr, uint16_t data);
	    void writeScrollY(bool upper, bool lower, uint32_t addr, uint16_t data);
	    void writeControl(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;

	    array<uint16_t, 4> control_words;

	    void renderBG();

	    array<uint16_t, 2> bg_scrollx;
	    array<uint16_t, 2> bg_scrolly;
	    array<uint16_t, 0x8000> vram;

	    array<int, 2> scrollx;
	    array<int, 2> scrolly;

	    array<pc080addr, 2> framebuffer;
    };
};

#endif // BERRN_PC080SN_H
