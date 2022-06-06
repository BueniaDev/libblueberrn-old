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

#ifndef BERRN_DRIVER
#define BERRN_DRIVER

#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <functional>
#include <utils.h>
#include <berrnfile.h>
#include <scheduler.h>
#include <graphics.h>
#include <libblueberrn_api.h>
namespace fs = std::filesystem;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    class LIBBLUEBERRN_API BlueberrnFrontend
    {
	public:
	    BlueberrnFrontend()
	    {
			
	    }
			
	    ~BlueberrnFrontend()
	    {
			
	    }
			
	    virtual bool init() = 0;
	    virtual void shutdown() = 0;
	    virtual void runapp() = 0;
	    virtual void resize(int width, int height, int scale) = 0;
	    virtual void drawpixels() = 0;
	    virtual int loadzip(string filename) = 0;
	    virtual vector<uint8_t> readfile(string dirname, string subdir_name, string filename) = 0;
	    virtual vector<uint8_t> readfilefromzip(int id, string dirname, string filename) = 0;
	    virtual void closezip() = 0;
	    virtual uint32_t getSampleRate() = 0;
	    virtual void audioCallback(array<int16_t, 2> samples) = 0;
    };

    enum BerrnInput
    {
	BerrnCoin,
	BerrnStartP1,
	BerrnLeftP1,
	BerrnRightP1,
	BerrnUpP1,
	BerrnDownP1,
	BerrnButton1P1,
	BerrnButton2P1,
	BerrnService,
	BerrnDump,
    };

    #define berrn_rom_erasemask    0x2000
    #define berrn_rom_erase        0x2000
    #define berrn_rom_erasevalmask 0x00FF0000
    #define berrn_rom_eraseval(x)  ((((x) & 0xFF) << 16) | berrn_rom_erase)
    #define berrn_rom_eraseff      berrn_rom_eraseval(0xFF)
    #define berrn_rom_erase00      berrn_rom_eraseval(0x00)

    #define berrn_is_erase      ((entry.flags & berrn_rom_erasemask) == berrn_rom_erase)
    #define berrn_get_erase_val ((entry.flags & berrn_rom_erasevalmask) >> 16)

    #define berrn_rom_groupmask  0x0F00
    #define berrn_rom_group_size(n)  ((((n) - 1) & 15) << 8)
    #define berrn_rom_group_byte     berrn_rom_group_size(1)
    #define berrn_rom_group_word     berrn_rom_group_size(2)
    #define berrn_get_group_size()   ((entry.flags & berrn_rom_groupmask) >> 8)

    #define berrn_rom_skipmask   0xF000
    #define berrn_rom_skip(n)    (((n) & 15) << 12)
    #define berrn_get_skip()     ((entry.flags & berrn_rom_skipmask) >> 12)

    struct berrnROMregion
    {
	string tagname;
	string filename;
	size_t offset;
	size_t length;
	uint32_t flags;
	bool is_directory;
    };

    struct berrnmemoryregion
    {
	string tagname;
	vector<uint8_t> data;
    };

    #define berrn_rom_name(name) rom_##name
    #define berrn_rom_start(name) static vector<berrnROMregion> berrn_rom_name(name) = { {"", #name, 0, 0, 0, true}, 
    #define berrn_rom_region(tag, length, flags) {tag, "", length, 0, flags, false},
    #define berrn_rom_load(filename, offs, length) {"", filename, offs, length, 0, false},
    #define berrn_rom_load16_byte(filename, offs, length) {"", filename, offs, length, berrn_rom_skip(1), false},
    #define berrn_rom_load32_byte(filename, offs, length) {"", filename, offs, length, berrn_rom_skip(3), false},
    #define berrn_rom_load32_word(filename, offs, length) {"", filename, offs, length, berrn_rom_group_word | berrn_rom_skip(2), false},
    #define berrn_rom_end {"", "", 0, 0, 0, false}};

    class LIBBLUEBERRN_API berrnROM
    {
	public:
	    berrnROM(BlueberrnFrontend *cb) : front(cb)
	    {

	    }

	    ~berrnROM()
	    {

	    }

	    vector<uint8_t> fetch_mem_region(string tagname)
	    {
		vector<uint8_t> temp;
		for (auto &region : memory_regions)
		{
		    if (region->tagname == tagname)
		    {
			temp = region->data;
			return temp;
		    }
		}

		return temp;
	    }

	    bool loadROM(vector<berrnROMregion> entries, string parent_name)
	    {
		auto entry_zero = entries.at(0);

		if (!entry_zero.is_directory)
		{
		    cout << "Error: First entry is not directory" << endl;
		    return false;
		}

		vector<string> dir_names = {entry_zero.filename, parent_name};

		is_dir_found = false;
		is_zip_found = false;
		string directory_name = "";
		string zip_name = "";

		for (auto &dir_name : dir_names)
		{
		    if (dir_name.empty())
		    {
			continue;
		    }

		    directory_name = get_path("roms", dir_name);
		    stringstream zip_name_stream;
		    zip_name_stream << directory_name << ".zip";
		    zip_name = zip_name_stream.str();

		    if (is_exists(directory_name) && is_rom_dir(directory_name))
		    {
			is_dir_found = true;
			break;
		    }
		    else if (is_exists(zip_name) && !is_rom_dir(zip_name))
		    {
			is_zip_found = true;
			break;
		    }
		}

		if (!is_dir_found && !is_zip_found)
		{
		    return false;
		}

		bool is_all_files_loaded = true;
		berrnmemoryregion *mem_region = NULL;

		for (size_t index = 1; index < entries.size(); index++)
		{
		    auto &entry = entries.at(index);

		    if (entry.filename.empty() && (entry.offset == 0) && (entry.length == 0))
		    {
			break;
		    }
		    else
		    {
			if (entry.filename.empty() && (entry.length == 0))
			{
			    uint8_t value = 0;

			    if (berrn_is_erase)
			    {
				value = berrn_get_erase_val;
			    }

			    mem_region = new berrnmemoryregion();
			    mem_region->tagname = entry.tagname;
			    mem_region->data.resize(entry.offset, value);
			    memory_regions.push_back(mem_region);
			}
			else
			{
			    vector<uint8_t> file_vec;

			    if (is_dir_found)
			    {
				file_vec = load_file(directory_name, entry_zero.filename, entry.filename);
			    }
			    else if (is_zip_found)
			    {
				file_vec = load_file_from_zip(zip_name, entry_zero.filename, entry.filename);
			    }

			    if (file_vec.empty())
			    {
				cout << "Error: Could not load file of " << entry.filename << endl;
				mem_region = NULL;
				memory_regions.clear();
				is_all_files_loaded = false;
				break;
			    }

			    size_t skip_bytes = berrn_get_skip();
			    size_t group_bytes = berrn_get_group_size();

			    if ((skip_bytes == 0) && (group_bytes == 0))
			    {
				auto databegin = file_vec.begin();
			    	auto dataend = (file_vec.begin() + entry.length);
			    	auto membegin = (mem_region->data.begin() + entry.offset);
			   	copy(databegin, dataend, membegin);
			    }
			    else
			    {
				size_t group_offs = (1 + group_bytes);
				size_t skip_offs = (skip_bytes + group_offs);
				size_t base_offs = entry.offset;

				for (size_t i = 0; i < entry.length; i += group_offs, base_offs += skip_offs)
				{
				    for (size_t j = 0; j < group_offs; j++)
				    {
					mem_region->data.at(base_offs + j) = file_vec.at(i + j);
				    }
				}
			    }
			}
		    }
		}

		if (is_zip_loaded)
		{
		    close_zip_internal();
		    is_zip_loaded = false;
		}

		return is_all_files_loaded;
	    }

	    void close_files()
	    {
		for (auto &region : memory_regions)
		{
		    region->data.clear();
		    region = NULL;
		}

		memory_regions.clear();
	    }

	private:
	    BlueberrnFrontend *front = NULL;

	    string get_path(const string subdir = "", const string romPath = "")
	    {
		fs::path cur_path = fs::current_path();

		cur_path.append(subdir);
		cur_path.append(romPath);

		return cur_path.generic_string();
	    }

	    string get_zip_path(const string subdir, const string romPath)
	    {
		fs::path zip_path;
		zip_path.append(subdir);
		zip_path.append(romPath);
		return zip_path.generic_string();
	    }

	    string get_zip_path(const string romPath)
	    {
		fs::path zip_path;
		zip_path.append(romPath);
		return zip_path.generic_string();
	    }

	    bool is_exists(string path_name)
	    {
		fs::path path = path_name;
		return fs::exists(path);
	    }

	    bool is_rom_dir(string dirname)
	    {
		fs::path path = dirname;
		return fs::is_directory(path);
	    }

	    bool is_file_exists(string file_name)
	    {
		return (is_exists(file_name) && !is_rom_dir(file_name));
	    }

	    vector<uint8_t> load_file(string dir_name, string subdir_name, string rom_name)
	    {
		string dir_path = get_path(dir_name);
		vector<uint8_t> data = load_file_internal(dir_name, subdir_name, rom_name);

		return data;
	    }

	    vector<uint8_t> load_file_from_zip(string zip_name, string subdir_name, string rom_name)
	    {
		vector<uint8_t> data;
		if (!is_zip_loaded)
		{
		    zip_id = load_zip_internal(zip_name);

		    if (zip_id == -1)
		    {
			return data;
		    }

		    is_zip_loaded = true;
		}

		data = read_file_zip_internal(zip_id, subdir_name, rom_name);

		return data;
	    }

	    vector<uint8_t> load_file_internal(string dirname, string subdirname, string filename)
	    {
		vector<uint8_t> data;

		if (front != NULL)
		{
		    data = front->readfile(dirname, subdirname, filename);
		}

		return data;
	    }

	    int load_zip_internal(string zip_name)
	    {
		if (front != NULL)
		{
		    return front->loadzip(zip_name);
		}

		return -1;
	    }

	    vector<uint8_t> read_file_zip_internal(int id, string dirname, string filename)
	    {
		vector<uint8_t> data;

		if (front != NULL)
		{
		    data = front->readfilefromzip(id, dirname, filename);
		}

		return data;
	    }

	    void close_zip_internal()
	    {
		if (front != NULL)
		{
		    front->closezip();
		}
	    }

	    vector<berrnmemoryregion*> memory_regions;
	    bool is_zip_loaded = false;
	    bool is_dir_found = false;
	    bool is_zip_found = false;
	    int zip_id = -1;
    };

    class LIBBLUEBERRN_API berrnmixer
    {
	public:
	    berrnmixer(BlueberrnFrontend *cb) : front(cb)
	    {

	    }

	    void add_mono(int32_t sample, double gain)
	    {
		int32_t adjusted_sample = int32_t(double(sample) * gain);
		mixed_samples[0] += adjusted_sample;
		mixed_samples[1] += adjusted_sample;
	    }

	    void add_stereo(int32_t left, int32_t right, double gain)
	    {
		array<int32_t, 2> samples = {left, right};
		add_stereo(samples, gain);
	    }

	    void add_stereo(array<int32_t, 2> samples, double gain)
	    {
		for (int i = 0; i < 2; i++)
		{
		    int32_t adjusted_sample = int32_t(double(samples[i]) * gain);
		    mixed_samples[i] += adjusted_sample;
		}
	    }

	    void output_sample()
	    {
		array<int16_t, 2> final_samples;

		for (int i = 0; i < 2; i++)
		{
		    final_samples[i] = clamp<int16_t>(mixed_samples[i], -32768, 32767);
		}

		if (front != NULL)
		{
		    front->audioCallback(final_samples);
		}

		mixed_samples.fill(0);
	    }

	private:
	    BlueberrnFrontend *front = NULL;

	    array<int32_t, 2> mixed_samples = {0, 0};
    };

    class LIBBLUEBERRN_API berrndriver
    {
	public:
	    berrndriver()
	    {
		mixer_timer = new BerrnTimer("AudioMixer", scheduler, [&](int64_t, int64_t) {
		    process_audio();
		    output_audio();
		});
	    }

	    ~berrndriver()
	    {

	    }

	    bool startdriver()
	    {
		cout << "Driver started" << endl;
		scheduler.reset();
		mixer_timer->start(time_in_hz(get_sample_rate()), true);
		return drvinit();
	    }

	    void stopdriver()
	    {
		drvshutdown();
		scheduler.shutdown();
		closedriver();
		cout << "Driver stopped" << endl;
	    }

	    void rundriver()
	    {
		drvrun();
	    }
      
	    void setfrontend(BlueberrnFrontend *cb)
	    {
		front = cb;
		rom_load = new berrnROM(front);
		mixer = new berrnmixer(front);
	    }
      
	    void resize(int width, int height, int scale = 1)
	    {
		if (front != NULL)
		{
		    front->resize(width, height, scale);
		}
	    }

	    BerrnBitmap *get_screen()
	    {
		return screen;
	    }

	    void set_screen(BerrnBitmap *bitmap)
	    {
		delete screen;
		if (bitmap->format() == BerrnRGB)
		{
		    BerrnBitmapRGB *bmp = reinterpret_cast<BerrnBitmapRGB*>(bitmap);
		    int rot_flags = (get_flags() & berrn_rot_mask);
		    screen = rotateBitmapRGB(bmp, rot_flags);
		}
		else
		{
		    screen = bitmap;
		}
	    }

	    uint32_t get_sample_rate()
	    {
		if (front == NULL)
		{
		    return 0;
		}

		return front->getSampleRate();
	    }

	    void add_mono_sample(int32_t sample, double gain = 1.0)
	    {
		mixer->add_mono(sample, gain);
	    }

	    void add_stereo_sample(int32_t left, int32_t right, double gain = 1.0)
	    {
		mixer->add_stereo(left, right, gain);
	    }

	    void output_audio()
	    {
		mixer->output_sample();
	    }

	    void drawpixels()
	    {
		if (front != NULL)
		{
		    front->drawpixels();
		}
	    }

	    bool loadROM(vector<berrnROMregion> entries)
	    {
		if (rom_load == NULL)
		{
		    return false;
		}

		return rom_load->loadROM(entries, parentname());
	    }

	    void closedriver()
	    {
		if (rom_load != NULL)
		{
		    rom_load->close_files();
		}
	    }

	    virtual uint32_t get_flags()
	    {
		return 0;
	    }

	    virtual void keychanged(BerrnInput key, bool is_pressed) = 0;

	    virtual string drivername() = 0;
	    virtual bool drvinit() = 0;
	    virtual void drvshutdown() = 0;
	    virtual void drvrun() = 0;

	    virtual string parentname()
	    {
		return "";
	    }

	    virtual void process_audio()
	    {
		add_stereo_sample(0, 0);
	    }

	    void run_scheduler()
	    {
		int64_t schedule_time = scheduler.get_current_time();
		int64_t frame_time = time_in_hz(get_framerate());

		while (scheduler.get_current_time() < (schedule_time + frame_time))
		{
		    scheduler.timeslice();
		}
	    }


	    virtual double get_framerate()
	    {
		return 60;
	    }

	    vector<uint8_t> get_rom_region(string tag)
	    {
		vector<uint8_t> temp;

		if (rom_load == NULL)
		{
		    return temp;
		}

		return rom_load->fetch_mem_region(tag);
	    }

	    BlueberrnFrontend *get_frontend()
	    {
		return front;
	    }

	    BerrnScheduler &get_scheduler()
	    {
		return scheduler;
	    }

	protected:
	    berrnmixer *mixer = NULL;

	private:
	    string romspath = "";
	    string dirname = "";

	    string samplespath = "";

	    BerrnBitmap *screen = NULL;

	    void closefiles()
	    {
		return;
	    }

	    vector<berrnmemoryregion*> memory_regions;
	    BlueberrnFrontend *front = NULL;
	    berrnROM *rom_load = NULL;

	    BerrnScheduler scheduler;
	    BerrnTimer *mixer_timer = NULL;
    };

    class LIBBLUEBERRN_API berrnaudiodevice
    {
	public:
	    berrnaudiodevice(berrndriver &drv) : driver(drv)
	    {

	    }

	    void init(uint32_t clk_rate)
	    {
		init_device();
		out_step = get_clock_rate(clk_rate);
		in_step = driver.get_sample_rate();
		out_time = 0.0f;
	    }

	    void init()
	    {
		init_device();
		out_step = driver.get_sample_rate();
		in_step = driver.get_sample_rate();
		out_time = 0.0f;
	    }

	    float get_sample_rate()
	    {
		return in_step;
	    }

	    vector<int32_t> fetch_samples()
	    {
		while (out_step > out_time)
		{
		    clock_chip();
		    out_time += in_step;
		}

		out_time -= out_step;

		vector<int32_t> samples = get_samples();
		return samples;
	    }

	protected:
	    virtual void init_device()
	    {
		return;
	    }

	    virtual void shutdown()
	    {
		return;
	    }

	    virtual uint32_t get_clock_rate(uint32_t clk_rate)
	    {
		return clk_rate;
	    }

	    virtual void clock_chip()
	    {
		return;
	    }

	    virtual vector<int32_t> get_samples()
	    {
		vector<int32_t> empty_samples;
		return empty_samples;
	    }

	private:
	    berrndriver &driver;
	    float out_step = 0.0f;
	    float in_step = 0.0f;
	    float out_time = 0.0f;
    };

    inline vector<berrndriver*> drivers;

    #define DRIVER(name) \
	driver##name *drv##name = new driver##name(); \
	driveradd(drv##name);

    inline void driveradd(berrndriver* drv)
    {
	drivers.push_back(drv);
    }

    inline berrndriver* searchdriver(string name)
    {
	for (auto& drv : drivers)
	{
	    if (drv->drivername() == name)
	    {
		return drv;
	    }
	}

	return NULL;
    }
  
    inline vector<string> getdrivernames()
    {
  	vector<string> temp;
  	for (auto& driver : drivers)
  	{
	    temp.push_back(driver->drivername());
  	}
  		
	return temp;
    }

    
};

#endif // BERRN_DRIVER