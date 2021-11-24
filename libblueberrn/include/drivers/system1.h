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

#ifndef BERRN_SEGASYS1
#define BERRN_SEGASYS1

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <machine/i8255.h>
#include <iostream>
#include <string>
#include <functional>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    using sys1inputfunc = function<uint8_t(int)>;
    using sys1outputfunc = function<void(int, uint8_t)>;
    // Interface for the main CPU
    class LIBBLUEBERRN_API Sys1MainInterface : public BerrnInterface
    {
	public:
	    Sys1MainInterface();
	    ~Sys1MainInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t readOp8(uint16_t addr);
	    uint8_t portIn(uint16_t port);
	    void portOut(uint16_t port, uint8_t data);

	    vector<uint8_t> &get_gamerom()
	    {
		return gamerom;
	    }

	    void setinputcallback(sys1inputfunc cb)
	    {
		inputcb = cb;
	    }

	    void setoutputcallback(sys1outputfunc cb)
	    {
		outputcb = cb;
	    }

	private:
	    uint8_t readByte(uint16_t addr);

	    vector<uint8_t> gamerom;
	    array<uint8_t, 0x1000> mainram;
	    array<uint8_t, 0x1000> vram;
	    array<uint8_t, 0x800> pram;
	    array<uint8_t, 0x800> oam;

	    sys1inputfunc inputcb;
	    sys1outputfunc outputcb;
    };

    // Core logic for the Sega System 1 hardware
    class LIBBLUEBERRN_API SegaSys1PPI
    {
	public:
	    SegaSys1PPI();
	    ~SegaSys1PPI();

	    void init();
	    void shutdown();
	    void run();

	    vector<uint8_t> &get_main_rom()
	    {
		return main_inter.get_gamerom();
	    }

	private:
	    Sys1MainInterface main_inter;
	    BerrnScheduler scheduler;

	    BerrnTimer *interrupt_timer = NULL;

	    // array<berrnRGBA, (512 * 224)> framebuffer;

	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    uint8_t readInput(int addr);
	    void writeOutput(int addr, uint8_t data);
	    void interruptHandler(int param);

	    i8255ppi main_ppi;
    };

    class LIBBLUEBERRN_API driverwboy2u : public berrndriver
    {
	public:
	    driverwboy2u();
	    ~driverwboy2u();

	    string drivername();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    SegaSys1PPI core_sys1;

	    void loadROMs();
    };
};


#endif // BERRN_SEGASYS1