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

#include "1943.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    static BerrnGfxLayout bg2_layout =
    {
	32, 32,
	128,
	4,
	{ 0x40004, 0x40000, 4, 0 },
	{ gfx_step4(0, 1),    gfx_step4(8, 1),    gfx_step4(512, 1),  gfx_step4(520, 1),
	  gfx_step4(1024, 1), gfx_step4(1032, 1), gfx_step4(1536, 1), gfx_step4(1544, 1) },
	{ gfx_step32(0, 16) },
	2048
    };
};