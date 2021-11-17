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

#ifndef LIBBLUEBERRN_MB14241_SHIFTER_H
#define LIBBLUEBERRN_MB14241_SHIFTER_H

#include <cstdint>
#include <iostream>
using namespace std;

namespace mb14241
{
    struct mb14241shifter
    {
	int shiftoffs = 0;
	uint16_t shiftval = 0;

	void setshiftoffs(uint8_t val)
	{
	    shiftoffs = (val & 0x07);
	}

	void fillshiftreg(uint8_t val)
	{
	    shiftval >>= 8;
	    shiftval |= (val << 8);
	}

	uint8_t readshiftresult()
	{
	    return (shiftval >> (8 - shiftoffs));
	}
    };
};

#endif // LIBBLUEBERRN_MB14241_SHIFTER_H