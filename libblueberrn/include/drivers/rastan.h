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

#ifndef BERRN_RASTAN
#define BERRN_RASTAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/motorola68k.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API RastanCore : public BerrnInterface
    {
	public:
	    RastanCore(berrndriver &drv);
	    ~RastanCore();

	    bool init_core();
	    void shutdown_core();
	    void run_core();

	    // Motorola 68000 memory functions
	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);
	    void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;

	    BerrnScheduler scheduler;
	    BerrnM68KProcessor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    // Placeholder bitmap
	    BerrnBitmapRGB *bitmap = NULL;

	    vector<uint8_t> m68k_rom;
	    array<uint8_t, 0x4000> m68k_ram;
    };

    class LIBBLUEBERRN_API driverrastan : public berrndriver
    {
	public:
	    driverrastan();
	    ~driverrastan();

	    string drivername();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    RastanCore *core = NULL;
    };
};

#endif // BERRN_RASTAN