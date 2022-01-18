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

#ifndef BERRN_INVADERS
#define BERRN_INVADERS

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/intel8080.h>
#include <machine/mb14241.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API InvadersCore : public BerrnInterface
    {
	public:
	    InvadersCore(berrndriver &drv);
	    ~InvadersCore();

	    bool init_core();
	    void shutdown_core();
	    void run_core();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t portIn(uint16_t port);
	    void portOut(uint16_t port, uint8_t val);

	    void key_changed(BerrnInput key, bool is_pressed);

	private:
	    berrndriver &driver;

	    BerrnScheduler scheduler;
	    Berrn8080Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    vector<int> sound_IDs;

	    void load_sound(string filename);

	    void play_sound(int id, bool is_playing = true);

	    void write_sound_port(int bank, uint8_t data);
	    bool is_rising_edge(uint8_t data, uint8_t prev_data, int bit);
	    bool is_falling_edge(uint8_t data, uint8_t prev_data, int bit);

	    mb14241shifter shifter;

	    uint8_t port1_val = 0;

	    uint8_t prev_port3 = 0;
	    uint8_t prev_port5 = 0;

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *sound_timer = NULL;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x400> work_ram;
	    array<uint8_t, 0x1C00> video_ram;

	    void update_pixels();

	    bool is_end_of_frame = false;

	    BerrnBitmapRGB *bitmap = NULL;

	    void debugPort(uint8_t val);
    };

    class LIBBLUEBERRN_API driverinvaders : public berrndriver
    {
	public:
	    driverinvaders();
	    ~driverinvaders();

	    string drivername();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    InvadersCore *inv_core = NULL;
    };
};

#endif // BERRN_INVADERS