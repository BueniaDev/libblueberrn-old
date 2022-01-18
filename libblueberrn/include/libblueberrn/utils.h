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

#ifndef BERRN_UTILS_H
#define BERRN_UTILS_H

#include <cstdint>
#include <functional>
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    using berrncbread8 = function<uint8_t(int)>;
    using berrncbwrite8 = function<void(int, uint8_t)>;

    template<typename T>
    bool testbit(T reg, int bit)
    {
	return ((reg >> bit) & 1) ? true : false;
    }

    template<typename T>
    T setbit(T reg, int bit)
    {
	return (reg | (1 << bit));
    }

    template<typename T>
    T resetbit(T reg, int bit)
    {
	return (reg & ~(1 << bit));
    }

    template<typename T>
    T changebit(T reg, int bit, bool val)
    {
	if (val == true)
	{
	    return setbit(reg, bit);
	}
	else
	{
	    return resetbit(reg, bit);
	}
    }

    template<typename T>
    bool inRange(T value, int low, int high)
    {
	int val = int(value);
	return ((val >= low) && (val < high));
    }
};

#endif // BERRN_UTILS_H