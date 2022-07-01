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

#ifndef BERRN_1943
#define BERRN_1943

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <video/1943.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class Berrn1943Core;

    class LIBBLUEBERRN_API Berrn1943Main : public BerrnInterface
    {
	public:
	    Berrn1943Main(berrndriver &drv, Berrn1943Core &core);
	    ~Berrn1943Main();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    Berrn1943Core &main_core;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x1000> main_ram;

	    // TODO: Finish implementing video hardware
	    array<uint8_t, 0x1000> sprite_ram;
	    array<uint8_t, 0x400> video_ram;
	    array<uint8_t, 0x400> color_ram;

	    int current_rom_bank = 0;
    };

    class LIBBLUEBERRN_API Berrn1943Core
    {
	public:
	    Berrn1943Core(berrndriver &drv);
	    ~Berrn1943Core();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    void writeIO(int addr, uint8_t data);

	    uint8_t readSecurity();
	    void writeSecurity(uint8_t data);

	private:
	    berrndriver &driver;

	    Berrn1943Main *main_inter = NULL;
	    BerrnZ80CPU *main_cpu = NULL;

	    berrn1943video *video = NULL;

	    void run_vblank(bool line);

	    uint8_t security_val = 0;

	    // TODO: Implement LLE of security MCU
	    unordered_map<uint8_t, uint8_t> security_hle =
	    {
		{0x24, 0x1D},
		{0x60, 0xF7},
		{0x01, 0xAC},
		{0x55, 0x50},
		{0x56, 0xE2},
		{0x2A, 0x58},
		{0xA8, 0x13},
		{0x22, 0x3E},
		{0x3B, 0x5A},
		{0x1E, 0x1B},
		{0xE9, 0x41},
		{0x7D, 0xD5},
		{0x43, 0x54},
		{0x37, 0x6F},
		{0x4C, 0x59},
		{0x5F, 0x56},
		{0x3F, 0x2F},
		{0x3E, 0x3D},
		{0xFB, 0x36},
		{0x1D, 0x3B},
		{0x27, 0xAE},
		{0x26, 0x39},
		{0x58, 0x3C},
		{0x32, 0x51},
		{0x1A, 0xA8},
		{0xBC, 0x33},
		{0x30, 0x4A},
		{0x64, 0x12},
		{0x11, 0x40},
		{0x33, 0x35},
		{0x09, 0x17},
		{0x25, 0x04}
	    };
    };

    class LIBBLUEBERRN_API driver1943u : public berrndriver
    {
	public:
	    driver1943u();
	    ~driver1943u();

	    string drivername();
	    string parentname();
	    uint32_t get_flags();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    Berrn1943Core *core = NULL;
    };
};


#endif // BERRN_1943