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

#ifndef BERRN_CAVE68K
#define BERRN_CAVE68K

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/motorola68k.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class CaveCore;

    class LIBBLUEBERRN_API CaveM68K : public BerrnInterface
    {
	public:
	    CaveM68K(berrndriver &drv, CaveCore &core);
	    ~CaveM68K();

	    virtual void init() = 0;
	    virtual void shutdown() = 0;
	    virtual uint16_t readCPU16(bool upper, bool lower, uint32_t addr) = 0;
	    virtual void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data) = 0;

	protected:
	    berrndriver &driver;
	    CaveCore &main_core;
    };

    class LIBBLUEBERRN_API DDonPachiM68K : public CaveM68K
    {
	public:
	    DDonPachiM68K(berrndriver &drv, CaveCore &core);
	    ~DDonPachiM68K();

	    void init();
	    void shutdown();
	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);
	    void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x10000> main_ram;

	    // TODO: Implement video system
	    array<uint8_t, 0x10000> sprite_ram;
    };

    class LIBBLUEBERRN_API CaveCore
    {
	public:
	    CaveCore(berrndriver &drv, CaveM68K &inter);
	    ~CaveCore();

	    void init_base(int time_irq = 100);
	    void stop_base();

	    virtual bool init_core();
	    virtual void stop_core();
	    void run_core();

	    virtual void vblank_callback(int time_irq);

	    void vblank_start();
	    void vblank_end();

	protected:
	    berrndriver &driver;
	    BerrnM68KCPU *main_cpu = NULL;
	    CaveM68K &interface;

	    void update_irq_state();

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *irq_timer = NULL;
	    BerrnTimer *vblank_end_timer = NULL;

	    bool is_vblank_irq = false;
    };

    class LIBBLUEBERRN_API DDonPachiCore : public CaveCore
    {
	public:
	    DDonPachiCore(berrndriver &drv);
	    ~DDonPachiCore();

	    bool init_core();
	    void stop_core();

	private:
	    DDonPachiM68K main_inter;
    };

    class LIBBLUEBERRN_API driverddonpach : public berrndriver
    {
	public:
	    driverddonpach();
	    ~driverddonpach();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    DDonPachiCore *core = NULL;
    };
};


#endif // BERRN_CAVE68K