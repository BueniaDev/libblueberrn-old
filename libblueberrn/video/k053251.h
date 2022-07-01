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

#ifndef BERRN_K053251_VIDEO_H
#define BERRN_K053251_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
#include <konami/k053251.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class k053251video
    {
	public:
	    k053251video(berrndriver &drv);
	    ~k053251video();

	    void init();
	    void write(int reg, uint8_t data);

	    void setPriority(int layer, uint8_t data);

	    void setInput(int layer, uint16_t data);
	    int getPaletteIndex(K053251Priority layer);
	    uint16_t getOutput(K053251Priority layer);

	    K053251Priority getLayer();

	private:
	    berrndriver &driver;

	    K053251 priority_chip;

	    array<int, 5> palette_index;
	    array<uint8_t, 5> layer_priorities;

	    array<bool, 3> is_priority_enabled;
	    array<uint8_t, 3> priority_inputs;

	    int getPriority(K053251Priority index);

	    void calcPriority();

	    array<int, 5> priority_layers;

	    array<uint16_t, 5> layer_inputs;

	    array<int, 5> prev_priorities;
    };
};

#endif // BERRN_K053251_VIDEO_H
