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

#ifndef LIBBLUEBERRN_UPD7759_H
#define LIBBLUEBERRN_UPD7759_H

#include <driver.h>
#include <pcm/BeePCM/BeePCM/uPD7759/upd7759.h>
using namespace beepcm;
using namespace berrn;

namespace berrn
{

    class upd7759device : public berrnaudiodevice
    {
	public:
	    upd7759device(berrndriver &drv) : berrnaudiodevice(drv), driver(drv)
	    {

	    }

	    void init_device()
	    {
		audio_core.init();
		setROM();
		cout << "uPD7759::Initialized..." << endl;
	    }

	    void shutdown()
	    {
		cout << "uPD7759::Shutting down..." << endl;
	    }

	    uint32_t get_clock_rate(uint32_t clk_rate)
	    {
		return audio_core.get_sample_rate(clk_rate);
	    }

	    bool read_busy()
	    {
		return audio_core.read_busy();
	    }

	    void write_start(bool line)
	    {
		audio_core.write_start(line);
	    }

	    void write_reset(bool line)
	    {
		audio_core.write_reset(line);
	    }

	    void write_port(uint8_t data)
	    {
		audio_core.write_port(data);
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
	    berrndriver &driver;
	    uPD7759 audio_core;

	    void setROM()
	    {
		auto rom_data = driver.get_rom_region("upd");
		audio_core.writeROM(rom_data);
	    }
    };
};


#endif // LIBBLUEBERRN_UPD7759_H