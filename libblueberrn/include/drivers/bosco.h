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

#ifndef BERRN_BOSCO
#define BERRN_BOSCO

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <machine/namco06.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class BoscoCore;

    class LIBBLUEBERRN_API BoscoInterface : public BerrnInterface
    {
	public:
	    BoscoInterface(berrndriver &drv, BoscoCore &core);
	    ~BoscoInterface();

	    void init(string tag);
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    BoscoCore &shared_core;

	    string tag_str;

	    vector<uint8_t> main_rom;
    };

    class LIBBLUEBERRN_API BoscoCore
    {
	public:
	    BoscoCore(berrndriver &drv);
	    ~BoscoCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    BoscoInterface *main_inter = NULL;
	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    namco06xx *n06xx_0 = NULL;
	    namco06xx *n06xx_1 = NULL;

	    // TODO: Implement video system
	    array<uint8_t, 0x1000> video_ram;

	    void writeIO(int addr, bool line);
    };

    class LIBBLUEBERRN_API driverbosco : public berrndriver
    {
	public:
	    driverbosco();
	    ~driverbosco();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    BoscoCore *core = NULL;
    };
};


#endif // BERRN_BOSCO