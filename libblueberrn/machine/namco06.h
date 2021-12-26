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

#ifndef BERRN_NAMCO06_H
#define BERRN_NAMCO06_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <scheduler.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class namco06xx
    {
	public:
	    namco06xx(BerrnCPU &cpu);
	    ~namco06xx();

	    uint8_t readControl();

	    void writeControl(uint8_t data);
	    void writeData(uint8_t data);

	private:
	    BerrnTimer *timer = NULL;
	    BerrnCPU &main_cpu;

	    uint8_t control_reg = 0;
    };
};


#endif // BERRN_NAMCO06_H