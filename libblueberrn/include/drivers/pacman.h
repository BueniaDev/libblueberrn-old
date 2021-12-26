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

#ifndef BERRN_PACMAN
#define BERRN_PACMAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <video/pacman.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API PacmanInterface : public BerrnInterface
    {
	public:
	    PacmanInterface(berrndriver &drv);
	    ~PacmanInterface();

	    bool init_core();
	    void shutdown_core();
	    void run_core();

	    void key_changed(BerrnInput key, bool is_pressed);

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t readOp8(uint16_t addr);
	    void portOut(uint16_t port, uint8_t data);

	private:
	    berrndriver &driver;

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    uint8_t readUpper(uint16_t addr);
	    void writeUpper(uint16_t addr, uint8_t data);

	    void writeIO(int addr, uint8_t data);

	    void writePort(uint16_t port, uint8_t data);

	    void update_pixels();

	    BerrnScheduler scheduler;
	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *vblank_timer = NULL;

	    pacmanvideo *video_core = NULL;

	    vector<uint8_t> game_rom;

	    array<uint8_t, 0x3F0> ram;

	    bool is_irq_enabled = false;

	    uint8_t irq_vector = 0;
    };

    class LIBBLUEBERRN_API driverpacman : public berrndriver
    {
	public:
	    driverpacman();
	    ~driverpacman();

	    string drivername();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    float get_framerate();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    PacmanInterface *inter = NULL;
    };
};


#endif // BERRN_PACMAN