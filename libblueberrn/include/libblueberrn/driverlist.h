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

#ifndef BERRN_DRIVERLIST
#define BERRN_DRIVERLIST

#include <berrndrivers.h>
#include <iostream>
#include <libblueberrn_api.h>
using namespace std;
using namespace berrn;

namespace berrn
{
    struct DriverList
    {
	void init()
	{
	    DRIVER(example);
	    DRIVER(invaders);
	    DRIVER(galaxian);
	    DRIVER(pacman);
	    DRIVER(galaga);
	    DRIVER(bosco);
	    DRIVER(wboy2u);
	    DRIVER(rastan); // WIP (relies on M68K emulation)
	    DRIVER(centiped);
	    DRIVER(1942);
	    DRIVER(1943u); // WIP
	    DRIVER(tmnt); // WIP (relies on M68K and K051960 emulation)
	    DRIVER(xmen); // WIP (relies on M68K emulation)
	    DRIVER(ddonpach); // WIP (relies on M68K emulation)
	    DRIVER(shuriboy);
	    DRIVER(mariorou);
	    DRIVER(aliens); // WIP (relies on KONAMI-2 emulation)
	    DRIVER(bombjack); // WIP
	}
    };
};

#endif // BERRN_DRIVERLIST