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

#ifndef BERRN_CENTIPEDE
#define BERRN_CENTIPEDE

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/mos6502.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API CentipedeBase : public BerrnInterface
    {
	public:
	    CentipedeBase(berrndriver &drv);
	    ~CentipedeBase();

	    bool init_core();
	    void shutdown_core();
	    void run_core();

	    void key_changed(BerrnInput key, bool is_pressed);

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	    virtual bool isValidAddr(uint16_t addr);
	    virtual uint8_t readMem(uint16_t addr);
	    virtual void writeMem(uint16_t addr, uint8_t data);

	    void writeLatch(int addr, uint8_t data);

	    virtual void writeLatchUpper(int addr, bool line);

	private:
	    berrndriver &driver;
	    BerrnScheduler scheduler;

	    Berrn6502Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *interrupt_timer = NULL;

	    BerrnBitmapRGB *bitmap = NULL;

	    array<uint8_t, 0x400> main_ram;

	    // TODO: Properly implement video hardware
	    array<uint8_t, 0x400> temp_vram;

	    int current_scanline = 0;

	    vector<uint8_t> main_rom;
    };

    class LIBBLUEBERRN_API CentipedeCore : public CentipedeBase
    {
	public:
	    CentipedeCore(berrndriver &drv);
	    ~CentipedeCore();

	    bool isValidAddr(uint16_t addr);
	    uint8_t readMem(uint16_t addr);
	    void writeMem(uint16_t addr, uint8_t data);

	    void writeLatchUpper(int addr, bool line);
    };

    class LIBBLUEBERRN_API drivercentiped : public berrndriver
    {
	public:
	    drivercentiped();
	    ~drivercentiped();

	    string drivername();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    CentipedeCore *core = NULL;
    };
};


#endif // BERRN_CENTIPEDE