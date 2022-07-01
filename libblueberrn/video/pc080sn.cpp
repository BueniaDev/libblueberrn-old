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

#include "pc080sn.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    pc080snvideo::pc080snvideo(berrndriver &drv) : driver(drv)
    {

    }

    pc080snvideo::~pc080snvideo()
    {

    }

    void pc080snvideo::init()
    {
	control_words.fill(0);
	vram.fill(0);

	for (int i = 0; i < 2; i++)
	{
	    framebuffer.at(i).fill(0);
	}
    }

    void pc080snvideo::shutdown()
    {
	return;
    }

    void pc080snvideo::render()
    {
	renderBG();
    }

    pc080addr pc080snvideo::getFramebuffer(int layer)
    {
	if ((layer < 0) || (layer >= 2))
	{
	    throw out_of_range("Invalid layer");
	}

	return framebuffer.at(layer);
    }

    void pc080snvideo::renderBG()
    {
	framebuffer.at(0).fill(0);
	for (int row = 0; row < 64; row++)
	{
	    for (int col = 0; col < 64; col++)
	    {
		int offs = ((row * 64) + col);

		uint16_t tile_num = (vram.at((offs * 2) + 1) & 0x3FFF);

		framebuffer[0].at(offs) = tile_num;
	    }
	}
    }

    uint16_t pc080snvideo::readRAM(bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;
	addr = ((addr >> 1) & 0x7FFF);

	uint16_t vram_data = vram.at(addr);

	if (upper)
	{
	    data |= (vram_data & 0xFF00);
	}

	if (lower)
	{
	    data |= (vram_data & 0xFF);
	}

	return data;
    }

    void pc080snvideo::writeRAM(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	addr = ((addr >> 1) & 0x7FFF);

	if (upper)
	{
	    vram.at(addr) = ((vram.at(addr) & 0xFF) | (data & 0xFF00));
	}

	if (lower)
	{
	    vram.at(addr) = ((vram.at(addr) & 0xFF00) | (data & 0xFF));
	}
    }

    void pc080snvideo::writeScrollX(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	addr = ((addr >> 1) & 0x1);
	int scrollx_data = bg_scrollx.at(addr);

	if (upper)
	{
	    scrollx_data = ((scrollx_data & 0xFF) | (data & 0xFF00));
	}

	if (lower)
	{
	    scrollx_data = ((scrollx_data & 0xFF00) | (data & 0xFF));
	}

	bg_scrollx.at(addr) = scrollx_data;

	int xscroll = int(scrollx_data);
	scrollx.at(addr) = -xscroll;
    }

    void pc080snvideo::writeScrollY(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	addr = ((addr >> 1) & 0x1);
	uint16_t scrolly_data = bg_scrolly.at(addr);

	if (upper)
	{
	    scrolly_data = ((scrolly_data & 0xFF) | (data & 0xFF00));
	}

	if (lower)
	{
	    scrolly_data = ((scrolly_data & 0xFF00) | (data & 0xFF));
	}

	bg_scrolly.at(addr) = scrolly_data;

	int yscroll = int(scrolly_data);
	scrolly.at(addr) = -yscroll;
    }

    void pc080snvideo::writeControl(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	addr = ((addr >> 1) & 0x1);

	uint16_t control_reg = control_words.at(addr);

	if (upper)
	{
	    control_reg = ((control_reg & 0xFF) | (data & 0xFF00));
	}

	if (lower)
	{
	    control_reg = ((control_reg & 0xFF00) | (data & 0xFF));
	}

	control_words.at(addr) = control_reg;

	switch (addr)
	{
	    case 0:
	    {
		bool is_flip = testbit(control_words.at(0), 0);

		if (is_flip)
		{
		    cout << "Screen is flipped" << endl;
		}
		else
		{
		    cout << "Screen is normal" << endl;
		}
	    }
	    break;
	}
    }
};