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

#ifndef LIBBLUEBERRN_AY8910_H
#define LIBBLUEBERRN_AY8910_H

#include <driver.h>
#include <psg/BeePSG/BeePSG/ay8910/ay8910.h>
using namespace beepsg;
using namespace berrn;

namespace berrn
{
    class ay8910device : public berrnaudiodevice
    {
	public:
	    ay8910device(berrndriver &drv) : berrnaudiodevice(drv)
	    {

	    }

	    void init_device()
	    {
		audio_core.init();
		cout << "AY8910::Initialized..." << endl;
	    }

	    void shutdown()
	    {
		cout << "AY8910::Shutting down..." << endl;
	    }

	    uint32_t get_clock_rate(uint32_t clk_rate)
	    {
		return audio_core.get_sample_rate(clk_rate);
	    }

	    void writeIO(int port, uint8_t data)
	    {
		audio_core.writeIO(port, data);
	    }

	    void clock_chip()
	    {
		audio_core.clock_chip();
	    }

	    vector<int32_t> get_samples()
	    {
		return audio_core.get_samples();
	    }

	private:
	    AY8910 audio_core;
    };
};


#endif // LIBBLUEBERRN_AY8910_H