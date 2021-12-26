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

#ifndef BERRN_DRIVER
#define BERRN_DRIVER

#include <iostream>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <utils.h>
#include <scheduler.h>
#include <graphics.h>
#include <libblueberrn_api.h>
namespace fs = std::filesystem;
using namespace std;

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
	    virtual vector<uint8_t> readfile(string filename) = 0;
	    virtual vector<uint8_t> readfilefromzip(int id, string filename) = 0;
	    virtual void closezip() = 0;
	    virtual uint32_t getSampleRate() = 0;
	    virtual int loadWAV(string filename) = 0;
	    virtual bool hasSounds() = 0;
	    virtual bool setSoundLoop(int id, bool is_loop) = 0;
	    virtual bool playSound(int id) = 0;
	    virtual bool stopSound(int id) = 0;
	    virtual bool setSoundVol(int id, double vol) = 0;
	    virtual array<int16_t, 2> getMixedSamples() = 0;
	    virtual void freeSounds() = 0;
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
	BerrnFireP1,
    };

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
    #define berrn_rom_region(tag, length) {tag, "", length, 0, 0, false},
    #define berrn_rom_load(filename, offs, length) {"", filename, offs, length, 0, false},
    #define berrn_rom_load16_byte(filename, offs, length) {"", filename, offs, length, berrn_rom_skip(1), false},
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

	    bool loadROM(vector<berrnROMregion> entries)
	    {
		bool is_roms_zipped = false;

		string directory_name;

		auto entry_zero = entries.at(0);

		if (!entry_zero.is_directory)
		{
		    cout << "Error: First entry is not directory" << endl;
		    return false;
		}

		directory_name = get_path("roms", entry_zero.filename);

		is_roms_zipped = !is_rom_dir(directory_name);

		int zip_id = -1;

		if (is_roms_zipped)
		{
		    zip_id = load_zip(directory_name);

		    if (zip_id == -1)
		    {
			return false;
		    }

		    is_zip_loaded = true;
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
			    mem_region = new berrnmemoryregion();
			    mem_region->tagname = entry.tagname;
			    mem_region->data.resize(entry.offset, 0);
			    memory_regions.push_back(mem_region);
			}
			else
			{
			    vector<uint8_t> file_vec;
			    if (is_roms_zipped)
			    {
				file_vec = load_file_from_zip(zip_id, entry.filename);
			    }
			    else
			    {
				file_vec = load_file(directory_name, entry.filename);
			    }

			    if (file_vec.empty())
			    {
				cout << "Error: Could not load file" << endl;
				mem_region = NULL;
				memory_regions.clear();
				is_all_files_loaded = false;
				break;
			    }

			    size_t skip_bytes = berrn_get_skip();

			    if (skip_bytes == 0)
			    {
				auto databegin = file_vec.begin();
			    	auto dataend = (file_vec.begin() + entry.length);
			    	auto membegin = (mem_region->data.begin() + entry.offset);
			   	copy(databegin, dataend, membegin);
			    }
			    else
			    {
				size_t skip_offs = (1 + skip_bytes);
				size_t base_offs = entry.offset;

				for (size_t i = 0; i < entry.length; i++, base_offs += skip_offs)
				{
				    mem_region->data[base_offs] = file_vec[i];
				}
			    }
			}
		    }
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

		if (is_zip_loaded)
		{
		    if (front != NULL)
		    {
			front->closezip();
		    }
		}
	    }

	private:
	    BlueberrnFrontend *front = NULL;

	    string get_path(const string subdir = "", const string romPath = "")
	    {
		fs::path cur_path = fs::current_path();

		cur_path.append(subdir);
		cur_path.append(romPath);
		return cur_path.string();
	    }

	    bool is_rom_dir(string dirname)
	    {
		fs::path path = dirname;
		return fs::is_directory(path);
	    }

	    int load_zip(string zipdir)
	    {
		if (front == NULL)
		{
		    return -1;
		}

		stringstream zip_str;
		zip_str << zipdir << ".zip";
		return front->loadzip(zip_str.str());
	    }

	    vector<uint8_t> load_file(string dirname, string filename)
	    {
		vector<uint8_t> file_vec;

		string full_filename = get_path(dirname, filename);

		if (front != NULL)
		{
		    file_vec = front->readfile(full_filename);
		}

		return file_vec;
	    }

	    vector<uint8_t> load_file_from_zip(int id, string filename)
	    {
		vector<uint8_t> file_vec;

		if (front != NULL)
		{
		    file_vec = front->readfilefromzip(id, filename);
		}

		return file_vec;
	    }

	    vector<berrnmemoryregion*> memory_regions;
	    bool is_zip_loaded = false;
	    bool isfilesloaded = false;
    };

    class LIBBLUEBERRN_API berrndriver
    {
	public:
	    berrndriver()
	    {

	    }

	    ~berrndriver()
	    {

	    }

	    bool startdriver()
	    {
		cout << "Driver started" << endl;
		final_samples.fill(0);
		return drvinit();
	    }

	    void stopdriver()
	    {
		drvshutdown();
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
	    }
      
	    void resize(int width, int height, int scale = 1)
	    {
		if (front != NULL)
		{
		    front->resize(width, height, scale);
		}
	    }

	    BerrnBitmap *getScreen()
	    {
		return screen;
	    }

	    void setScreen(BerrnBitmap *bitmap)
	    {
		screen = bitmap;
	    }

	    bool setSoundLoop(int id, bool is_loop)
	    {
		if (front == NULL)
		{
		    return false;
		}

		return front->setSoundLoop(id, is_loop);
	    }

	    int loadWAV(string filename)
	    {
		return loadSoundWAV(filename);
	    }

	    bool setSoundVol(int id, double vol)
	    {
		if (front == NULL)
		{
		    return false;
		}

		return front->setSoundVol(id, vol);
	    }

	    bool playSound(int id)
	    {
		if (front == NULL)
		{
		    return false;
		}

		return front->playSound(id);
	    }

	    bool stopSound(int id)
	    {
		if (front == NULL)
		{
		    return false;
		}

		return front->stopSound(id);
	    }

	    uint32_t getSampleRate()
	    {
		if (front == NULL)
		{
		    return 0;
		}

		return front->getSampleRate();
	    }

	    array<int16_t, 2> getRawSample()
	    {
		if (front == NULL)
		{
		    return {0, 0};
		}

		if (!front->hasSounds())
		{
		    return {0, 0};
		}

		return front->getMixedSamples();
	    }

	    void mixSample(array<int16_t, 2> samples)
	    {
		int32_t current_left = final_samples[0];
		int32_t current_right = final_samples[1];

		int32_t new_left = samples[0];
		int32_t new_right = samples[1];

		int32_t mixed_left = (current_left + new_left);
		int32_t mixed_right = (current_right + new_right);

		final_samples = {int16_t(mixed_left), int16_t(mixed_right)};
	    }

	    void outputAudio()
	    {
		if (front == NULL)
		{
		    return;
		}

		front->audioCallback(final_samples);
		final_samples.fill(0);
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

		return rom_load->loadROM(entries);
	    }

	    void closedriver()
	    {
		closeSounds();
		if (rom_load != NULL)
		{
		    rom_load->close_files();
		}
	    }

	    virtual void keychanged(BerrnInput key, bool is_pressed) = 0;

	    virtual string drivername() = 0;
	    virtual bool hasdriverROMs() = 0;
	    virtual bool drvinit() = 0;
	    virtual void drvshutdown() = 0;
	    virtual void drvrun() = 0;

	    virtual float get_framerate()
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

	private:
	    bool is_dir_check = false;
	    bool is_dir_exists = false;

	    bool is_samples_dir = false;
	    bool is_samples_dir_exists = false;

	    string romspath = "";
	    string dirname = "";

	    string samplespath = "";

	    BerrnBitmap *screen = NULL;

	    array<int16_t, 2> final_samples = {0, 0};

	    int loadSoundWAV(string filename)
	    {
		if (front == NULL)
		{
		    return -1;
		}

		fs::path cur_path = fs::current_path();
		cur_path.append("samples");
		cur_path.append(drivername());
		cur_path.append(filename);

		return front->loadWAV(cur_path.string());
	    }

	    void closeSounds()
	    {
		if (front != NULL)
		{
		    front->freeSounds();
		}
	    }

	    void closefiles()
	    {
		return;
	    }

	    vector<berrnmemoryregion*> memory_regions;
	    BlueberrnFrontend *front = NULL;
	    berrnROM *rom_load = NULL;
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