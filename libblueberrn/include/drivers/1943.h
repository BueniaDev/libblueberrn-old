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
#include <iostream>
#include <string>
#include <cpu/zilogz80.h>
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

	    // TODO: Implement video system
	    array<uint8_t, 0x1000> obj_ram;
	    array<uint8_t, 0x400> video_ram;
	    array<uint8_t, 0x400> color_ram;
	    uint16_t scrollx = 0;
	    uint8_t scrolly = 0;
	    uint16_t bg_scrollx = 0;
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

	private:
	    berrndriver &driver;
	    Berrn1943Main *main_inter = NULL;
	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    BerrnTimer *irq_timer = NULL;
    };

    class LIBBLUEBERRN_API driver1943u : public berrndriver
    {
	public:
	    driver1943u();
	    ~driver1943u();

	    string drivername();
	    string parentname();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    Berrn1943Core *core = NULL;
    };
};


#endif // BERRN_1943