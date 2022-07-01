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

#ifndef LIBBLUEBERRN_WSG_AUDIO_H
#define LIBBLUEBERRN_WSG_AUDIO_H

#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

namespace namcowsg
{
    class wsgaudio
    {
	public:
	    wsgaudio()
	    {

	    }

	    ~wsgaudio()
	    {

	    }

	    uint32_t get_sample_rate(uint32_t clock_rate)
	    {
		return clock_rate;
	    }

	    // Initialize the chip (and set the sound ROM)
	    void init(vector<uint8_t> waveROM)
	    {
		soundROM = vector<uint8_t>(waveROM);
		init_voices();
		cout << "NamcoWSG3::Initialized" << endl;
	    }

	    void shutdown()
	    {
		voices.clear();
		soundROM.clear();
		cout << "NamcoWSG3::Shutting down..." << endl;
	    }

	    void set_num_voices(int num)
	    {
		if (num_voices != num)
		{
		    num_voices = num;
		    init_voices();
		}
	    }

	    // Write to a WSG3 register
	    void write_wsg3(int addr, uint8_t data)
	    {
		addr &= 0x1F;
		data &= 0xF;

		switch (addr)
		{
		    case 0x05: write_waveform_wsg3(0, data); break;
		    case 0x0A: write_waveform_wsg3(1, data); break;
		    case 0x0F: write_waveform_wsg3(2, data); break;
		    case 0x10: write_frequency_wsg3(0, 0, data); break;
		    case 0x11: write_frequency_wsg3(0, 1, data); break;
		    case 0x12: write_frequency_wsg3(0, 2, data); break;
		    case 0x13: write_frequency_wsg3(0, 3, data); break;
		    case 0x14: write_frequency_wsg3(0, 4, data); break;
		    case 0x15: write_volume_wsg3(0, data); break;
		    case 0x16: write_frequency_wsg3(1, 1, data); break;
		    case 0x17: write_frequency_wsg3(1, 2, data); break;
		    case 0x18: write_frequency_wsg3(1, 3, data); break;
		    case 0x19: write_frequency_wsg3(1, 4, data); break;
		    case 0x1A: write_volume_wsg3(1, data); break;
		    case 0x1B: write_frequency_wsg3(2, 1, data); break;
		    case 0x1C: write_frequency_wsg3(2, 2, data); break;
		    case 0x1D: write_frequency_wsg3(2, 3, data); break;
		    case 0x1E: write_frequency_wsg3(2, 4, data); break;
		    case 0x1F: write_volume_wsg3(2, data); break;
		}
	    }

	    // Enables or disables sound
	    void set_sound_enabled(bool val)
	    {
		is_sound_enabled = val;
	    }

	    // Clock the chip once
	    void clockchip()
	    {
		for (auto &voice : voices)
		{
		    voice.accumulator = ((voice.accumulator + voice.frequency_reg) & 0xFFFFF);
		    int sample_pos = ((voice.waveform_sel * 32) + ((voice.accumulator >> 15) & 0x1F));
		    voice.sample = ((soundROM.at(sample_pos) - 8) * voice.volume_reg);
		}
	    }

	    int32_t get_sample()
	    {
		int32_t sample = 0;
		if (is_sound_enabled)
		{
		    for (auto &voice : voices)
		    {
			sample += voice.sample;
		    }

		    return (sample * 25);
		}

		return 0;
	    }

	private:
	    vector<uint8_t> soundROM;

	    struct wsgvoice
	    {
		int waveform_sel = 0;
		int volume_reg = 0;
		int frequency_reg = 0;
		int accumulator = 0;
		int32_t sample = 0;
	    };

	    vector<wsgvoice> voices;

	    void init_voices()
	    {
		voices.clear();
		voices.resize(num_voices, {0, 0, 0, 0, 0});
	    }

	    void write_waveform_wsg3(int voice, int data)
	    {
		if ((voice < 0) || (voice >= num_voices))
		{
		    return;
		}

		auto &channel = voices.at(voice);
		channel.waveform_sel = (data & 0x7);
	    }

	    void write_volume_wsg3(int voice, int data)
	    {
		if ((voice < 0) || (voice >= num_voices))
		{
		    return;
		}

		auto &channel = voices.at(voice);
		channel.volume_reg = (data & 0xF);
	    }

	    void write_frequency_wsg3(int voice, int offs, int data)
	    {
		if ((voice < 0) || (voice >= num_voices))
		{
		    return;
		}

		auto &channel = voices.at(voice);
		int bit_offs = (offs << 2);
		int bit_mask = (0xF << bit_offs);
		channel.frequency_reg = ((channel.frequency_reg & ~bit_mask) | ((data & 0xF) << bit_offs));
	    }

	    bool is_sound_enabled = false;
	    int num_voices = 0;
    };
};

#endif // LIBBLUEBERRN_WSG_AUDIO_H