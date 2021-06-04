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
    using invsoundfunc = function<void(uint8_t, bool)>;

    class LIBBLUEBERRN_API InvadersInterface : public Bee8080Interface
    {
	public:
	    InvadersInterface();
	    ~InvadersInterface();

	    invsoundfunc soundfunc;

	    void init();
	    void shutdown();
	    void reset();
	    void run();
	    void updatepixels(vector<berrnRGBA> &buffer);
	    void setsoundfunc(invsoundfunc cb);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t val);
	    uint8_t portIn(uint8_t port);
	    void portOut(uint8_t port, uint8_t val);

	    void loadSound(int id);

	    void playSound(uint8_t val, bool bank);

	    void coin(bool pressed);
	    void startp1(bool pressed);
	    void leftp1(bool pressed);
	    void rightp1(bool pressed);
	    void firep1(bool pressed);

	    vector<int> soundIDs;

	    vector<uint8_t> gamerom;

	    vector<uint8_t> workram;
	    vector<uint8_t> videoram;

	private:
	    const int cpu_frequency = 2000000; // 2 MHz clock speed
	    const int video_frequency = 60; // 60 FPS
	    const int cycles_per_half_frame = (cpu_frequency / video_frequency) / 2; // 2 MHz at 60 fps, divided by two

	    const uint8_t interrupt0 = 0xCF; // RST 1 at the start of VBlank
	    const uint8_t interrupt1 = 0xD7; // RST 2 at the end of VBlank

	    const int width = 256;
	    const int height = 224;

	    void runHalfFrame();

	    int totalcycles = 0;

	    void changePort1Bit(int bit, bool pressed);

	    void debugPort(uint8_t val);

	    uint8_t port1 = 0;

	    Bee8080 core;
	    mb14241shifter shifter;
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

	    void drvcoin(bool pressed);
	    void drvstartp1(bool pressed);
	    void drvleftp1(bool pressed);
	    void drvrightp1(bool pressed);
	    void drvfirep1(bool pressed);

	    bitset<4> prev_port3;

	    bitset<5> prev_port5;

	    void invadersSound(uint8_t val, bool bank);

	    void loadInvSound(string filename);
	    void playInvSound(int id);

	    vector<int> soundIDs;

	    InvadersInterface inter;

	    vector<berrnRGBA> framebuffer;
    };
};

#endif // BERRN_INVADERS