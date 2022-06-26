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

#ifndef BERRN_PUNKSHOT
#define BERRN_PUNKSHOT

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/motorola68k.h>
#include <video/punkshot.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class PunkshotCore;

    class LIBBLUEBERRN_API PunkshotM68K : public BerrnInterface
    {
	public:
	    PunkshotM68K(berrndriver &drv, PunkshotCore &core);
	    ~PunkshotM68K();

	    void init();
	    void shutdown();

	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);
	    void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;
	    PunkshotCore &main_core;

	    vector<uint8_t> main_rom;

	    array<uint8_t, 0x4000> main_ram;
    };

    class LIBBLUEBERRN_API PunkshotCore
    {
	public:
	    PunkshotCore(berrndriver &drv);
	    ~PunkshotCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    void writeA0020(uint8_t data);

	    uint16_t tile_read(bool upper, bool lower, uint32_t addr);
	    void tile_write(bool upper, bool lower, uint32_t addr, uint16_t data);

	    uint8_t sprite_read(uint16_t addr);
	    void sprite_write(uint16_t addr, uint8_t data);

	    void priority_write(uint32_t addr, uint8_t data);

	    uint16_t palette_read(bool upper, bool lower, uint32_t addr);
	    void palette_write(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;
	    PunkshotM68K *main_inter = NULL;
	    BerrnM68KCPU *main_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;

	    punkshotvideo *video = NULL;
    };

    class LIBBLUEBERRN_API driverpunkshot : public berrndriver
    {
	public:
	    driverpunkshot();
	    ~driverpunkshot();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    PunkshotCore *core = NULL;
    };
};


#endif // BERRN_PUNKSHOT