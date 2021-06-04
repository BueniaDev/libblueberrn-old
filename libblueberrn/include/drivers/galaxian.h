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

#ifndef BERRN_GALAXIAN
#define BERRN_GALAXIAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API GalaxianInterface : public BeeZ80Interface
    {
	public:
	    GalaxianInterface();
	    ~GalaxianInterface();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t val);
	    uint8_t portIn(uint8_t port);
	    void portOut(uint8_t port, uint8_t val);

	    void init();
	    void shutdown();
	    void reset();
	    void run();

	    vector<uint8_t> gamerom;

	private:
	    BeeZ80 core;

	    const int cpu_frequency = 3072000; // 3.072 MHz clock speed
	    const int video_frequency = 60; // 60 FPS
	    const int cycles_per_frame = (cpu_frequency / video_frequency); // 3.072 MHz at 60 fps

	    int totalcycles = 0;
    };

    class LIBBLUEBERRN_API drivergalaxian : public berrndriver
    {
	public:
	    drivergalaxian();
	    ~drivergalaxian();

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

	private:
	    GalaxianInterface inter;
    };
};


#endif // BERRN_GALAXIAN