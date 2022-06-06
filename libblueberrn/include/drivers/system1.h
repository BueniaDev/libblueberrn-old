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

#ifndef BERRN_SYS1
#define BERRN_SYS1

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <machine/i8255.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class SegaSystem1;

    class LIBBLUEBERRN_API Sys1MainInterface : public BerrnInterface
    {
	public:
	    Sys1MainInterface(berrndriver &drv, SegaSystem1 &core);
	    ~Sys1MainInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t portIn(uint16_t port);
	    void portOut(uint16_t port, uint8_t data);

	private:
	    berrndriver &driver;
	    SegaSystem1 &main_core;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x1000> main_ram;

	    // TODO: Implement video system
	    array<uint8_t, 0x1000> video_ram;

	    array<uint8_t, 0x800> sprite_ram;

	    array<uint8_t, 0x800> palette_ram;
    };

    class LIBBLUEBERRN_API SegaSystem1
    {
	public:
	    SegaSystem1(berrndriver &drv);
	    ~SegaSystem1();

	    virtual bool init_core();
	    virtual void stop_core();
	    void run_core();

	    virtual uint8_t portIn(uint16_t addr);
	    virtual void portOut(uint16_t addr, uint8_t data);

	    virtual uint8_t readDIP(int bank);

	private:
	    berrndriver &driver;
	    Sys1MainInterface *main_inter = NULL;

	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;
    };

    class LIBBLUEBERRN_API SegaSys1PPI : public SegaSystem1
    {
	public:
	    SegaSys1PPI(berrndriver &drv);
	    ~SegaSys1PPI();

	    bool init_core();
	    void stop_core();

	    uint8_t portIn(uint16_t addr);
	    void portOut(uint16_t addr, uint8_t data);

	private:
	    i8255ppi main_ppi;
    };

    class LIBBLUEBERRN_API WonderBoyCore : public SegaSys1PPI
    {
	public:
	    WonderBoyCore(berrndriver &drv);
	    ~WonderBoyCore();

	    uint8_t readDIP(int bank);
    };

    class LIBBLUEBERRN_API driverwboy2u : public berrndriver
    {
	public:
	    driverwboy2u();
	    ~driverwboy2u();

	    string drivername();
	    string parentname();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    WonderBoyCore *core = NULL;
    };
};

#endif // BERRN_SYS1