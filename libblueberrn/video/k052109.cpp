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

#include "k052109.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    k052109video::k052109video(berrndriver &drv) : driver(drv)
    {

    }

    void k052109video::init()
    {
	tile_chip.init();
	gfx_chip.init();
    }

    void k052109video::setROM(vector<uint8_t> &gfx_rom)
    {
	tile_rom = vector<uint8_t>(gfx_rom.begin(), gfx_rom.end());
	tile_chip.set_gfx_rom(tile_rom);
	gfx_chip.set_gfx_rom(tile_rom);
    }

    void k052109video::shutdown()
    {
	tile_rom.clear();
	stopLog();
    }

    array<int, (512 * 256)> k052109video::render(int layer)
    {
	if (!inRange(layer, 0, 3))
	{
	    throw out_of_range("Invalid layer number");
	}

	auto tile_addr = tile_chip.render(layer);
	return gfx_chip.render(layer, tile_addr);
    }

    void k052109video::setCallback(k052109callback cb)
    {
	tilecb = cb;

	auto tile_func = [&](uint8_t code, uint8_t attrib, int bank) -> uint32_t
	{
	    uint32_t data = 0;

	    if (tilecb)
	    {
		data = tilecb(code, attrib, bank);
	    }

	    return data;
	};

	tile_chip.set_tile_read_cb(tile_func);
	gfx_chip.set_tile_callback(tile_func);
    }

    void k052109video::logRender()
    {
	log_render();
    }

    void k052109video::startLog(string filename)
    {
	open_file(filename);
	write_header();
    }

    void k052109video::stopLog()
    {
	log_end();
	close_file();
    }

    uint8_t k052109video::read(uint16_t addr)
    {
	return tile_chip.read(addr);
    }

    void k052109video::write(uint16_t addr, uint8_t data)
    {
	log_write(addr, data);
	bool is_ben = tile_chip.write(addr, data);

	if (is_ben)
	{
	    gfx_chip.write(data);
	}
    }

    void k052109video::setRMRD(bool line)
    {
	log_rmrd(line);
	tile_chip.set_rmrd_line(line);
    }
};