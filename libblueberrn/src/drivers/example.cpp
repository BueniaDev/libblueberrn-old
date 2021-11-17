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

#include "example.h"
using namespace berrn;

namespace berrn
{
    driverexample::driverexample()
    {

    }

    driverexample::~driverexample()
    {

    }

    string driverexample::drivername()
    {
	return "example";
    }

    bool driverexample::hasdriverROMs()
    {
	return false;
    }

    bool driverexample::drvinit()
    {
	resize(640, 480, 1);
	return true;
    }

    void driverexample::drvshutdown()
    {
	return;
    }
  
    void driverexample::drvrun()
    {
	fillrect(0, 0, 640, 480, red());
	fillrect(50, 50, 100, 100, green());
    }

    void driverexample::keychanged(BerrnInput key, bool is_pressed)
    {
	return;
    }
};