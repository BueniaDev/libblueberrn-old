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

#include "k053251.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    k053251video::k053251video(berrndriver &drv) : driver(drv)
    {

    }

    k053251video::~k053251video()
    {

    }

    void k053251video::init()
    {
	priority_chip.init();
    }

    void k053251video::write(int reg, uint8_t data)
    {
	priority_chip.write(reg, data);
    }

    void k053251video::setPriority(int layer, uint8_t data)
    {
	priority_chip.set_priority(layer, data);
    }

    void k053251video::setInput(int layer, uint16_t data)
    {
	priority_chip.set_input(layer, data);
    }

    int k053251video::getPaletteIndex(K053251Priority layer)
    {
	return priority_chip.get_palette_index(layer);
    }

    uint16_t k053251video::getOutput(K053251Priority layer)
    {
	return priority_chip.get_output(layer);
    }

    K053251Priority k053251video::getLayer()
    {
	return priority_chip.get_top_layer();
    }
};