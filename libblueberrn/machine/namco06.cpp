/*
    This file is part of libblueberrn.
    Copyright (C) 2021 BueniaDev.

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

#include "namco06.h"
using namespace berrn;

namespace berrn
{
    namco06xx::namco06xx(BerrnCPU &cpu) : main_cpu(cpu)
    {
	timer = new BerrnTimer("GenNMI", cpu.get_scheduler(), [&](int64_t, int64_t) {
	    cpu.get_processor().fire_nmi();
	});
    }

    namco06xx::~namco06xx()
    {

    }

    void namco06xx::writeData(uint8_t data)
    {
	if (testbit(control_reg, 4))
	{
	    cout << "Writing in read-only mode" << endl;
	    return;
	}

	for (int i = 0; i < 4; i++)
	{
	    if (testbit(control_reg, i))
	    {
		cout << "Writing value of " << hex << int(data) << " to Namco 06xx interface device " << dec << int(i) << endl;
	    }
	}
    }

    uint8_t namco06xx::readControl()
    {
	return control_reg;
    }

    void namco06xx::writeControl(uint8_t data)
    {
	control_reg = data;
	if ((data & 0xF) == 0)
	{
	    timer->stop();
	}
	else
	{
	    timer->start(200, true); // Time is in microseconds
	}
    }
};