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

#include "pacman.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout charlayout = 
    {
	8, 8,
	256,
	2,
	{0, 4},
	{ gfx_step4(64, 1), gfx_step4(0, 1) },
	{ gfx_step8(0, 8) },
	128
    };

    static BerrnGfxLayout spritelayout = 
    {
	16, 16,
	64,
	2,
	{0, 4},
	{ gfx_step4(64, 1), gfx_step4(128, 1), gfx_step4(192, 1), gfx_step4(0, 1) },
	{ gfx_step8(0, 8), gfx_step8(256, 8) },
	512
    };
};