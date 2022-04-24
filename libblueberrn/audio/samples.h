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

#ifndef LIBBLUEBERRN_SAMPLES_H
#define LIBBLUEBERRN_SAMPLES_H

#include <driver.h>
using namespace berrn;

namespace berrn
{
    #define berrn_fx_bits 12
    #define berrn_fx_unit (1 << berrn_fx_bits)
    #define berrn_fx_mask (berrn_fx_unit - 1)
    #define berrn_fx_from_float(f) ((f) * berrn_fx_unit)
    #define berrn_fx_lerp(a, b, p) ((a) + ((((b) - (a)) * (p)) >> berrn_fx_bits))

    class samplesdevice : public berrnaudiodevice
    {
	public:
	    samplesdevice(berrndriver &drv) : berrnaudiodevice(drv), driver(drv)
	    {

	    }

	    void init_device()
	    {
		front = driver.get_frontend();
		num_files = 0;
	    }

	    void shutdown()
	    {
		for (auto &source : sample_files)
		{
		    source.is_paused = true;
		    source.buffer.clear();
		}

		sample_files.clear();
		num_files = 0;
		front = NULL;
	    }

	    void clock_chip()
	    {
		for (auto &source : sample_files)
		{
		    if (!source.is_paused)
		    {
			size_t frame = (source.buffer_pos >> berrn_fx_bits);

			if (frame >= source.end_pos)
			{
			    source.buffer_pos = 0;

			    if (!source.is_loop)
			    {
				source.is_paused = true;
				continue;
			    }
			}

			int num = ((source.buffer_pos >> berrn_fx_bits) * 2);
			int frac = (source.buffer_pos & berrn_fx_mask);

			for (int j = 0; j < 2; j++)
			{
			    size_t curr_index = (num + j);
			    size_t next_index = ((num + 2 + j) % source.buffer_size);
			    int sample1 = source.buffer.at(curr_index);
			    int sample2 = source.buffer.at(next_index);

			    source.output[j] = berrn_fx_lerp(sample1, sample2, frac);
			}

			source.buffer_pos += source.playback_rate;
		    }
		}
	    }

	    vector<int32_t> get_samples()
	    {
		array<int32_t, 2> output = {0, 0};

		for (auto &source : sample_files)
		{
		    output[0] += source.output[0];
		    output[1] += source.output[1];
		}

		vector<int32_t> samples;
		samples.push_back(output[0]);
		samples.push_back(output[1]);
		return samples;
	    }

	    int load_sample(string filename)
	    {
		if (front == NULL)
		{
		    cout << "Frontend is NULL" << endl;
		    return -1;
		}

		fs::path cur_path = fs::current_path();
		cur_path.append("samples");
		cur_path.append(driver.drivername());
		cur_path.append(filename);

		vector<uint8_t> file_vec = front->readfile(cur_path.string());

		if (file_vec.empty())
		{
		    cout << "Could not load file of " << filename << endl;
		    return -1;
		}

		berrnsource sample;

		BerrnFile sample_file;
		sample_file.open(file_vec);

		bool is_error = false;

		string riff_tag = sample_file.read_fourcc();

		if (riff_tag == "RIFF")
		{
		    if (!read_wav_sample(sample_file, sample))
		    {
			is_error = true;
		    }
		}
		else
		{
		    is_error = true;
		}

		if (is_error)
		{
		    sample_file.close();
		    return -1;
		}

		size_t file_id = num_files;
		sample_files.push_back(sample);
		num_files = sample_files.size();
		sample_file.close();
		return file_id;
	    }

	    bool set_sound_loop(int sample_num, bool is_loop)
	    {
		if (!inRange(sample_num, 0, num_files))
		{
		    return false;
		}

		sample_files.at(sample_num).is_loop = is_loop;
		return true;
	    }

	    bool play_sound(int sample_num)
	    {
		if (!inRange(sample_num, 0, num_files))
		{
		    return false;
		}

		sample_files.at(sample_num).is_paused = false;
		return true;
	    }
	    
	    bool stop_sound(int sample_num)
	    {
		if (!inRange(sample_num, 0, num_files))
		{
		    return false;
		}

		sample_files.at(sample_num).is_paused = true;
		return true;
	    }

	protected:
	    struct berrnsource
	    {
		vector<int16_t> buffer;
		size_t buffer_size = 0;
		size_t buffer_pos = 0;
		uint32_t sample_rate = 0;
		size_t end_pos = 0;
		int playback_rate = 0;
		bool is_loop = false;
		bool is_paused = false;
		array<int32_t, 2> output = {0, 0};
	    };

	private:
	    berrndriver &driver;
	    vector<berrnsource> sample_files;
	    BlueberrnFrontend *front = NULL;
	    size_t num_files = 0;

	    bool read_wav_sample(BerrnFile &file, berrnsource &sample)
	    {
		uint32_t file_size = file.read_u32_le();

		if (file_size < 36)
		{
		    cout << "Invalid file size" << endl;
		    return false;
		}

		string wave_tag = file.read_fourcc();

		if (wave_tag != "WAVE")
		{
		    cout << "Invalid wave file" << endl;
		    return false;
		}

		string tag_val = "";
		uint32_t chunk_size = 0;

		while (true)
		{
		    tag_val = file.read_fourcc();
		    chunk_size = file.read_u32_le();

		    if (tag_val == "fmt ")
		    {
			break;
		    }

		    file.advance_bytes(chunk_size);

		    if (file.has_ended())
		    {
			cout << "Could not find format subchunk" << endl;
			return false;
		    }
		}

		uint16_t audio_format = file.read_u16_le();

		if (audio_format != 1)
		{
		    cout << "Invalid audio format - only PCM supported" << endl;
		    return false;
		}

		uint16_t num_channels = file.read_u16_le();

		if ((num_channels != 1) && (num_channels != 2))
		{
		    cout << "Invalid number of channels - only mono and stereo supported" << endl;
		    return false;
		}

		uint32_t sample_rate = file.read_u32_le();

		file.advance_bytes(6);

		uint16_t num_bits = file.read_u16_le();

		if ((num_bits != 8) && (num_bits != 16))
		{
		    cout << "Invalid bits/sample - only 8 and 16-bits supported" << endl;
		    return false;
		}

		file.advance_bytes((chunk_size - 16));

		while (true)
		{
		    tag_val = file.read_fourcc();
		    chunk_size = file.read_u32_le();

		    if (tag_val == "data")
		    {
			break;
		    }

		    file.advance_bytes(chunk_size);

		    if (file.has_ended())
		    {
			cout << "Could not find format subchunk" << endl;
			return false;
		    }
		}

		uint32_t num_samples = (chunk_size / num_channels / (num_bits / 8));

		sample.buffer.resize((num_samples * 2), 0);
		sample.buffer_size = sample.buffer.size();
		sample.buffer_pos = 0;
		sample.sample_rate = sample_rate;
		sample.end_pos = num_samples;
		sample.playback_rate = berrn_fx_from_float(float(sample_rate) / get_sample_rate());
		sample.is_loop = false;
		sample.is_paused = true;
		sample.output.fill(0);

		if ((num_bits == 16) && (num_channels == 2))
		{
		    for (size_t i = 0; i < num_samples; i++)
		    {
			size_t frame = (i * 2);
			sample.buffer.at(frame) = fetch_i16(file);
			sample.buffer.at(frame + 1) = fetch_i16(file);
		    }
		}
		else if ((num_bits == 16) && (num_channels == 1))
		{
		    for (size_t i = 0; i < num_samples; i++)
		    {
			size_t frame = (i * 2);

			int16_t sample_i16 = fetch_i16(file);
			sample.buffer.at(frame) = sample_i16;
			sample.buffer.at(frame + 1) = sample_i16;
		    }
		}
		else if ((num_bits == 8) && (num_channels == 2))
		{
		    for (size_t i = 0; i < num_samples; i++)
		    {
			size_t frame = (i * 2);
			sample.buffer.at(frame) = fetch_u8(file);
			sample.buffer.at(frame + 1) = fetch_u8(file);
		    }
		}
		else if ((num_bits == 8) && (num_channels == 1))
		{
		    for (size_t i = 0; i < num_samples; i++)
		    {
			size_t frame = (i * 2);

			int16_t sample_i16 = fetch_u8(file);
			sample.buffer.at(frame) = sample_i16;
			sample.buffer.at(frame + 1) = sample_i16;
		    }
		}

		return true;
	    }

	    int16_t fetch_u8(BerrnFile &file)
	    {
		uint8_t sample_u8 = file.read_u8();
		return int16_t(int8_t(sample_u8 ^ 0x80) << 8);
	    }

	    int16_t fetch_i16(BerrnFile &file)
	    {
		uint16_t sample_u16 = file.read_u16_le();
		return int16_t(sample_u16);
	    }
    };
};


#endif // LIBBLUEBERRN_SAMPLES_H