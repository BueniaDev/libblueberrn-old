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

#ifndef BERRN_GALAXIAN
#define BERRN_GALAXIAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <iostream>
#include <string>
#include <cpu/zilogz80.h>
#include <video/galaxian.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API GalaxianCore : public BerrnInterface
    {
	public:
	    GalaxianCore(berrndriver &drv);
	    ~GalaxianCore();

	    bool initcore();
	    void stopcore();
	    void runcore();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x400> main_ram;

	    void writeIOUpper(int reg, bool line);

	    bool irq_enable = false;

	    BerrnZ80CPU *main_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;

	    galaxianvideo *video = NULL;
    };

    class LIBBLUEBERRN_API drivergalaxian : public berrndriver
    {
	public:
	    drivergalaxian();
	    ~drivergalaxian();

	    string drivername();
	    uint32_t get_flags();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    GalaxianCore *core = NULL;
    };
};


#endif // BERRN_GALAXIAN