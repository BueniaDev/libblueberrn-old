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

#ifndef LIBBLUEBERRN_K007232_H
#define LIBBLUEBERRN_K007232_H

#include <driver.h>
#include "konami/BeeKonami/BeeKonami/audio/K007232/k007232.h"
using namespace beekonami::audio;
using namespace berrn;

namespace berrn
{
    using k007232volume = function<void(uint8_t)>;

    class k007232device : public berrnaudiodevice
    {
	public:
	    k007232device(berrndriver &drv) : berrnaudiodevice(drv), driver(drv)
	    {

	    }

	    void init_device()
	    {
		audio_core.init();
		setROM();
		cout << "K007232::Initialized..." << endl;
	    }

	    void shutdown()
	    {
		cout << "K007232::Shutting down..." << endl;
	    }

	    void setVolumeCallback(k007232volume cb)
	    {
		volumecb = cb;
	    }

	    uint32_t get_clock_rate(uint32_t clk_rate)
	    {
		return audio_core.get_sample_rate(clk_rate);
	    }

	    uint8_t readReg(int reg)
	    {
		return audio_core.readReg(reg);
	    }

	    void writeReg(int reg, uint8_t data)
	    {
		bool is_vol = audio_core.writeReg(reg, data);

		if (is_vol == true)
		{
		    if (volumecb)
		    {
			volumecb(data);
		    }
		}
	    }

	    void setVolume(int vol_a, int vol_b)
	    {
		audio_core.setVolume(vol_a, vol_b);
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
	    berrndriver &driver;
	    K007232 audio_core;

	    k007232volume volumecb;

	    void setROM()
	    {
		auto rom_data = driver.get_rom_region("k007232");
		audio_core.writeROM(rom_data);
	    }
    };
}

#endif // LIBBLUEBERRN_K007232_H