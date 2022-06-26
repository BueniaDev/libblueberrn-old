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
	    DRIVER(example); // Example driver
	    DRIVER(invaders); // Playable, but no sound
	    DRIVER(galaxian); // No sprites, input or sound
	    DRIVER(pacman); // No input or sound
	    DRIVER(galaga); // WIP (relies on several Namco custom chips)
	    DRIVER(bosco); // WIP (relies on several Namco custom chips)
	    DRIVER(wboy2u); // WIP (TODO: Implement graphics support)
	    DRIVER(rastan); // WIP (TODO: Finish PC080SN implementation)
	    DRIVER(centiped); // WIP (TODO: Implement Pokey core for libblueberrn)
	    DRIVER(1942); // Missing a few inputs, but otherwise should be functional
	    DRIVER(1943u); // WIP
	    DRIVER(tmnt); // Should be fully playable
	    DRIVER(tmntu); // Should be fully playable
	    DRIVER(tmht); // Should be fully playable
	    DRIVER(mia); // Should be fully playable
	    DRIVER(punkshot); // No input or sound
	    DRIVER(xmen); // WIP (relies on M68K emulation)
	    DRIVER(ddonpach); // WIP (relies on M68K emulation)
	    DRIVER(shuriboy); // No input or sound
	    DRIVER(mariorou); // No input or sound
	    DRIVER(aliens); // WIP (relies on KONAMI-2 emulation)
	    DRIVER(blockhl); // WIP (relies on KONAMI-2 emulation)
	    DRIVER(bombjack); // Missing inputs
	    DRIVER(gradius); // WIP
	}
    };
};

#endif // BERRN_DRIVERLIST