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

#ifndef LIBBLUEBERRN_WSG3_AUDIO_H
#define LIBBLUEBERRN_WSG3_AUDIO_H

#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

namespace namcowsg3
{
    class wsg3audio
    {
	public:
	    wsg3audio()
	    {

	    }

	    ~wsg3audio()
	    {

	    }

	    uint32_t get_sample_rate()
	    {
		// The Namco WSG3's internal sample rate is 96 kHz
		return 96000;
	    }

	    // Initialize the chip (and set the sound ROM)
	    void init(vector<uint8_t> waveROM)
	    {
		soundROM = vector<uint8_t>(waveROM);
		accumulators.fill(0);
		frequencies.fill(0);
		waveforms.fill(0);
		volumes.fill(0);
		outputs.fill(0);
		cout << "NamcoWSG3::Initialized" << endl;
	    }

	    void shutdown()
	    {
		soundROM.clear();
		cout << "NamcoWSG3::Shutting down..." << endl;
	    }

	    // Write to a register
	    void writereg(int addr, uint8_t data)
	    {
		// Note: The lower 4 bits of both the accumulator and frequency
		// of voices 2 and 3 are unused

		// Only the lower 4 bits of the 8-bit data are used
		int value = (data & 0xF);
		switch ((addr & 0x1F))
		{
		    case 0x00:
		    {
			accumulators[0] = ((accumulators[0] & 0xFFFF0) | value);
		    }
		    break;
		    case 0x01:
		    {
			accumulators[0] = ((accumulators[0] & 0xFFF0F) | (value << 4));
		    }
		    break;
		    case 0x02:
		    {
			accumulators[0] = ((accumulators[0] & 0xFF0FF) | (value << 8));
		    }
		    break;
		    case 0x03:
		    {
			accumulators[0] = ((accumulators[0] & 0xF0FFF) | (value << 12));
		    }
		    break;
		    case 0x04:
		    {
			accumulators[0] = ((accumulators[0] & 0x0FFFF) | (value << 16));
		    }
		    break;
		    // Voice 1 waveform
		    case 0x05:
		    {
			waveforms[0] = (value & 0x7);
		    }
		    break;
		    case 0x06:
		    {
			accumulators[1] = ((accumulators[1] & 0xFFF0F) | (value << 4));
		    }
		    break;
		    case 0x07:
		    {
			accumulators[1] = ((accumulators[1] & 0xFF0FF) | (value << 8));
		    }
		    break;
		    case 0x08:
		    {
			accumulators[1] = ((accumulators[1] & 0xF0FFF) | (value << 12));
		    }
		    break;
		    case 0x09:
		    {
			accumulators[1] = ((accumulators[1] & 0x0FFFF) | (value << 16));
		    }
		    break;
		    // Voice 2 waveform
		    case 0x0A:
		    {
			waveforms[1] = (value & 0x7);
		    }
		    break;
		    case 0x0B:
		    {
			accumulators[2] = ((accumulators[2] & 0xFFF0F) | (value << 4));
		    }
		    break;
		    case 0x0C:
		    {
			accumulators[2] = ((accumulators[2] & 0xFF0FF) | (value << 8));
		    }
		    break;
		    case 0x0D:
		    {
			accumulators[2] = ((accumulators[2] & 0xF0FFF) | (value << 12));
		    }
		    break;
		    case 0x0E:
		    {
			accumulators[2] = ((accumulators[2] & 0x0FFFF) | (value << 16));
		    }
		    break;
		    // Voice 3 waveform
		    case 0x0F:
		    {
			waveforms[2] = (value & 0x7);
		    }
		    break;
		    case 0x10:
		    {
			frequencies[0] = ((frequencies[0] & 0xFFFF0) | value);
		    }
		    break;
		    case 0x11:
		    {
			frequencies[0] = ((frequencies[0] & 0xFFF0F) | (value << 4));
		    }
		    break;
		    case 0x12:
		    {
			frequencies[0] = ((frequencies[0] & 0xFF0FF) | (value << 8));
		    }
		    break;
		    case 0x13:
		    {
			frequencies[0] = ((frequencies[0] & 0xF0FFF) | (value << 12));
		    }
		    break;
		    case 0x14:
		    {
			frequencies[0] = ((frequencies[0] & 0x0FFFF) | (value << 16));
		    }
		    break;
		    case 0x15:
		    {
			volumes[0] = value;
		    }
		    break;
		    case 0x16:
		    {
			frequencies[1] = ((frequencies[1] & 0xFFF0F) | (value << 4));
		    }
		    break;
		    case 0x17:
		    {
			frequencies[1] = ((frequencies[1] & 0xFF0FF) | (value << 8));
		    }
		    break;
		    case 0x18:
		    {
			frequencies[1] = ((frequencies[1] & 0xF0FFF) | (value << 12));
		    }
		    break;
		    case 0x19:
		    {
			frequencies[1] = ((frequencies[1] & 0x0FFFF) | (value << 16));
		    }
		    break;
		    case 0x1A:
		    {
			volumes[1] = value;
		    }
		    break;
		    case 0x1B:
		    {
			frequencies[2] = ((frequencies[2] & 0xFFF0F) | (value << 4));
		    }
		    break;
		    case 0x1C:
		    {
			frequencies[2] = ((frequencies[2] & 0xFF0FF) | (value << 8));
		    }
		    break;
		    case 0x1D:
		    {
			frequencies[2] = ((frequencies[2] & 0xF0FFF) | (value << 12));
		    }
		    break;
		    case 0x1E:
		    {
			frequencies[2] = ((frequencies[2] & 0x0FFFF) | (value << 16));
		    }
		    break;
		    case 0x1F:
		    {
			volumes[2] = value;
		    }
		    break;
		    default: break;
		}
	    }

	    // Clock the chip once
	    void clockchip()
	    {
		for (int i = 0; i < 3; i++)
		{
		    // Don't calculate a channel's output if its frequency or its volume is 0
		    if ((frequencies[i] == 0) || (volumes[i] == 0))
		    {
			continue;
		    }

		    // Add the frequency to the 20-bit accumulator
		    accumulators[i] = ((accumulators[i] + frequencies[i]) & 0xFFFFF);
		    // Calculate the sound index using the top 5 bits of the accumulator
		    int sound_index = ((waveforms[i] << 5) + (accumulators[i] >> 15));
		    // Convert the corresponding byte in the sound ROM to a 16-bit signed sample
		    // and multiply it by the volume
		    outputs[i] = ((soundROM[sound_index] - 8) * volumes[i]);
		}
	    }

	    array<int16_t, 2> get_sample()
	    {
		int32_t sample = 0;

		// Mix the individual outputs together
		for (int i = 0; i < 3; i++)
		{
		    sample += outputs[i];
		}

		// Multiply the final sample by a gain of 25...
		int16_t mixed_sample = (sample * 25);

		// ...and convert the mono sample to a stereo one
		array<int16_t, 2> final_samples = {mixed_sample, mixed_sample};
		return final_samples;
	    }

	private:
	    vector<uint8_t> soundROM;

	    array<uint32_t, 3> accumulators;
	    array<int, 3> waveforms;
	    array<int, 3> volumes;
	    array<uint32_t, 3> frequencies;
	    array<int32_t, 3> outputs;
    };
};

#endif // LIBBLUEBERRN_WSG3_AUDIO_H