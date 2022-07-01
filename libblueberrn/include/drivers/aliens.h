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

#ifndef BERRN_ALIENS
#define BERRN_ALIENS

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/konami2.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class AliensCore;

    class LIBBLUEBERRN_API AliensMainInterface : public BerrnInterface
    {
	public:
	    AliensMainInterface(berrndriver &drv, AliensCore &core);
	    ~AliensMainInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    AliensCore &main_core;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x1C00> main_ram;
    };

    class LIBBLUEBERRN_API AliensCore
    {
	public:
	    AliensCore(berrndriver &drv);
	    ~AliensCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    uint8_t readBank0000(uint16_t addr);
	    void writeBank0000(uint16_t addr, uint8_t data);

	    void writeCoinCounter(uint8_t data);

	private:
	    berrndriver &driver;

	    AliensMainInterface *main_inter = NULL;
	    BerrnKonami2CPU *main_cpu = NULL;

	    bool is_bank_0000_ram = false;

	    array<uint8_t, 0x400> palette_ram;
	    array<uint8_t, 0x400> bank_0000_ram;
    };

    class LIBBLUEBERRN_API driveraliens : public berrndriver
    {
	public:
	    driveraliens();
	    ~driveraliens();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    AliensCore *core = NULL;
    };
};


#endif // BERRN_ALIENS