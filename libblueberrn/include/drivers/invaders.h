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
    using invoutputfunc = function<void(int, bool)>;

    class LIBBLUEBERRN_API InvadersInterface : public BerrnInterface
    {
	public:
	    InvadersInterface();
	    ~InvadersInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t portIn(uint16_t port);
	    void portOut(uint16_t port, uint8_t val);
	    void updatePixels();

	    void setsoundcallback(invoutputfunc cb)
	    {
		invoutput = cb;
	    }

	    vector<uint8_t> &get_gamerom()
	    {
		return gamerom;
	    }

	    array<berrnRGBA, (256 * 224)> get_framebuffer() const
	    {
		return framebuffer;
	    }

	    void coin(bool is_pressed);
	    void p1start(bool is_pressed);
	    void p1left(bool is_pressed);
	    void p1right(bool is_pressed);
	    void p1fire(bool is_pressed);

	private:
	    vector<uint8_t> gamerom;
	    vector<uint8_t> workram;
	    vector<uint8_t> videoram;

	    invoutputfunc invoutput;

	    uint8_t prev_port3 = 0;
	    uint8_t prev_port5 = 0;

	    void write_sound_port(int bank, uint8_t val);

	    void play_sound(int id, bool is_playing = true);

	    void debugPort(uint8_t val);

	    uint8_t port1_val = 0;

	    mb14241shifter shifter;

	    array<berrnRGBA, (256 * 224)> framebuffer;

	    const size_t width = 224;
	    const size_t height = 256;
    };

    class LIBBLUEBERRN_API driverinvaders : public berrndriver
    {
	public:
	    driverinvaders();
	    ~driverinvaders();

	    virtual string drivername();
	    virtual void loadROMs();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	    void interrupt_handler();

	private:
	    vector<int> sound_IDs;

	    InvadersInterface inter;
	    BerrnScheduler scheduler;

	    void sound_handler(int id, bool is_playing);
	    void load_sound(string filename);

	    Berrn8080Processor *invaders_proc = NULL;
	    BerrnCPU *invaders_cpu = NULL;
	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *sound_timer = NULL;

	    bool is_end_of_frame = false;
    };
};

#endif // BERRN_INVADERS