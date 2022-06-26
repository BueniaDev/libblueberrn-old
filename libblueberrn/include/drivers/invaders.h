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

#ifndef BERRN_INVADERS
#define BERRN_INVADERS

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/intel8080.h>
#include <machine/mb14241.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API InvadersCore : public BerrnInterface
    {
	public:
	    InvadersCore(berrndriver &drv);
	    ~InvadersCore();

	    bool initcore();
	    void stopcore();
	    void runcore();
	    void keychanged(BerrnInput key, bool is_pressed);

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t portIn(uint16_t port);
	    void portOut(uint16_t port, uint8_t data);

	private:
	    berrndriver &driver;

	    Berrn8080CPU *main_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *irq_timer = NULL;

	    vector<uint8_t> rom;
	    array<uint8_t, 0x400> main_ram;
	    array<uint8_t, 0x1C00> video_ram;

	    uint8_t port1 = 0;

	    bool is_end_of_frame = false;

	    BerrnBitmapRGB *bitmap = NULL;

	    void debugPort(uint8_t data);

	    void updatePixels();

	    mb14241shifter shifter;
    };

    class LIBBLUEBERRN_API driverinvaders : public berrndriver
    {
	public:
	    driverinvaders();
	    ~driverinvaders();

	    string drivername();
	    uint32_t get_flags();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    InvadersCore *core = NULL;
    };
};

#endif // BERRN_INVADERS