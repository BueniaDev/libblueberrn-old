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

#ifndef BERRN_RASTAN
#define BERRN_RASTAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/motorola68k.h>
#include <video/rastan.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class RastanCore;

    class RastanM68K : public BerrnInterface
    {
	public:
	    RastanM68K(berrndriver &drv, RastanCore &core);
	    ~RastanM68K();

	    void init();
	    void shutdown();

	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);
	    void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x4000> main_ram;


	    berrndriver &driver;
	    RastanCore &main_core;
    };

    class LIBBLUEBERRN_API RastanCore
    {
	public:
	    RastanCore(berrndriver &drv);
	    ~RastanCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    uint16_t readPC080SN(int bank, bool upper, bool lower, uint32_t addr);
	    void writePC080SN(int bank, bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;

	    RastanM68K *main_inter = NULL;
	    BerrnM68KCPU *main_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;

	    rastanvideo *tile_video = NULL;
    };

    class LIBBLUEBERRN_API driverrastan : public berrndriver
    {
	public:
	    driverrastan();
	    ~driverrastan();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    RastanCore *core = NULL;
    };
};

#endif // BERRN_RASTAN