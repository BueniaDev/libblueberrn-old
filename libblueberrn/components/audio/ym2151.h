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

#ifndef LIBBLUEBERRN_YM2151_H
#define LIBBLUEBERRN_YM2151_H

#include <driver.h>
#include <fm/BeeNuked/BeeNuked/OPM/ym2151.h>
using namespace beenuked;
using namespace berrn;

namespace berrn
{
    class ym2151device : public berrnaudiodevice
    {
	public:
	    ym2151device(berrndriver &drv) : berrnaudiodevice(drv)
	    {

	    }

	    void init_device()
	    {
		audio_core.init();
		cout << "YM2151::Initialized..." << endl;
	    }

	    void shutdown()
	    {
		cout << "YM2151::Shutting down..." << endl;
	    }

	    uint32_t get_clock_rate(uint32_t clk_rate)
	    {
		return audio_core.get_sample_rate(clk_rate);
	    }

	    uint8_t readIO(int port)
	    {
		return audio_core.readIO(port);
	    }

	    void writeIO(int port, uint8_t data)
	    {
		audio_core.writeIO(port, data);

		if (port == 0)
		{
		    opm_reg = data;
		}
		else if (port == 1)
		{
		    if ((opm_reg != 0x12) && (opm_reg != 0x14))
		    {
			cout << "Writing value of " << hex << int(data) << " to YM2151 register of " << hex << int(opm_reg) << endl;
		    }
		}
	    }

	    void clock_chip()
	    {
		audio_core.clockchip();
	    }

	    vector<int32_t> get_samples()
	    {
		return audio_core.get_samples();
	    }

	private:
	    YM2151 audio_core;

	    uint8_t opm_reg = 0;
    };

    namespace beenukedutil
    {
	#include <fm/BeeNuked/BeeNuked/OPM/ym3014.inl>
    };
};


#endif // LIBBLUEBERRN_YM2151_H