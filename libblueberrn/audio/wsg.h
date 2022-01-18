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
		waveROM = driver.get_rom_region(tag);
		audio_core.init(waveROM);
		audio_core.set_num_voices(3);
	    }

	    void shutdown()
	    {
		audio_core.shutdown();
	    }

	    void write_reg(int addr, uint8_t data)
	    {
		audio_core.write_wsg3(addr, data);
	    }

	    void set_sound_enabled(bool is_enabled)
	    {
		audio_core.set_sound_enabled(is_enabled);
	    }

	    uint32_t get_clock_rate(uint32_t clk_rate)
	    {
		return audio_core.get_sample_rate(clk_rate);
	    }

	    void clock_chip()
	    {
		audio_core.clockchip();
	    }

	    vector<int32_t> get_samples()
	    {
		vector<int32_t> samples;
		samples.push_back(audio_core.get_sample());
		return samples;
	    }

	private:
	    wsgaudio audio_core;

	    berrndriver &driver;
	    vector<uint8_t> waveROM;
    };
};

#endif // LIBBLUEBERRN_WSG_H