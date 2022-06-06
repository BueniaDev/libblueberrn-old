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

#ifndef LIBBLUEBERRN_WSG_H
#define LIBBLUEBERRN_WSG_H

#include <driver.h>
#include "wsg/wsg.h"
using namespace namcowsg;
using namespace berrn;

namespace berrn
{
    class wsg3device : public berrnaudiodevice
    {
	public:
	    wsg3device(berrndriver &drv) : driver(drv)
	    {

	    }

	    void init(string tag = "namco")
	    {
		cout << "Fetching wave ROM with tag of " << tag << endl;
	    }

	    void shutdown()
	    {
		return;
	    }

	    void write_reg(int addr, uint8_t data)
	    {
		cout << "Writing value of " << hex << int(data) << " to Namco WSG3 address of " << hex << int(addr) << endl;
	    }

	    void set_sound_enabled(bool is_enabled)
	    {
		if (is_enabled)
		{
		    cout << "Sound enabled" << endl;
		}
		else
		{
		    cout << "Sound disabled" << endl;
		}
	    }

	    uint32_t get_clock_rate(uint32_t clk_rate)
	    {
		return clk_rate;
	    }

	    void clock_chip()
	    {
		return;
	    }

	    vector<int32_t> get_samples()
	    {
		vector<int32_t> samples;
		samples.push_back(0);
		return samples;
	    }

	private:
	    berrndriver &driver;
    };
};

#endif // LIBBLUEBERRN_WSG_H