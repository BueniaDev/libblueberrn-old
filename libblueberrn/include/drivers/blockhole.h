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

#ifndef BERRN_BLOCKHOLE
#define BERRN_BLOCKHOLE

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/konami2.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class BlockHoleCore;

    class LIBBLUEBERRN_API BlockHoleMainInterface : public BerrnInterface
    {
	public:
	    BlockHoleMainInterface(berrndriver &drv, BlockHoleCore &core);
	    ~BlockHoleMainInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    // SETLINES callback for KONAMI-2 core
	    void portOut(uint16_t port, uint8_t data);

	private:
	    berrndriver &driver;
	    BlockHoleCore &main_core;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x1800> main_ram;

	    int current_rom_bank = 0;
    };

    class LIBBLUEBERRN_API BlockHoleCore
    {
	public:
	    BlockHoleCore(berrndriver &drv);
	    ~BlockHoleCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    void setLines(uint8_t data);

	    uint8_t readDIP(int bank);

	    uint8_t readBank5800(uint16_t addr);
	    void writeBank5800(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    BlockHoleMainInterface *main_inter = NULL;
	    BerrnKonami2CPU *main_cpu = NULL;

	    bool is_bank_5800_ram = false;

	    array<uint8_t, 0x800> palette_ram;
	    array<uint8_t, 0x800> bank_5800_ram;
    };

    class LIBBLUEBERRN_API driverblockhl : public berrndriver
    {
	public:
	    driverblockhl();
	    ~driverblockhl();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    BlockHoleCore *core = NULL;
    };
};


#endif // BERRN_BLOCKHOLE