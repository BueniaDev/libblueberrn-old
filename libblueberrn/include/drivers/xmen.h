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

#ifndef BERRN_XMEN
#define BERRN_XMEN

#include <libblueberrn_api.h>
#include <driver.h>
#include <iostream>
#include <string>
#include <cpu/motorola68k.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class XMenCore;

    class XMenM68K : public BerrnInterface
    {
	public:
	    XMenM68K(berrndriver &drv, XMenCore &core);
	    ~XMenM68K();

	    void init();
	    void shutdown();

	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);
	    void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;
	    XMenCore &main_core;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x4000> main_ram;
    };

    class XMenCore
    {
	public:
	    XMenCore(berrndriver &drv);
	    ~XMenCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    void writeIRQ(bool line);

	private:
	    berrndriver &driver;

	    XMenM68K *main_inter = NULL;
	    BerrnM68KProcessor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *irq_timer = NULL;

	    int current_scanline = 0;

	    bool is_vblank_irq = false;
    };

    class LIBBLUEBERRN_API driverxmen : public berrndriver
    {
	public:
	    driverxmen();
	    ~driverxmen();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    XMenCore *core = NULL;
    };
};


#endif // BERRN_XMEN