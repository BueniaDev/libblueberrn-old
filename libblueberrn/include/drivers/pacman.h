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

#ifndef BERRN_PACMAN
#define BERRN_PACMAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <video/pacman.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API PacmanCore : public BerrnInterface
    {
	public:
	    PacmanCore(berrndriver &drv);
	    ~PacmanCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    void portOut(uint16_t port, uint8_t data);

	private:
	    berrndriver &driver;

	    // BerrnZ80Processor *main_proc = NULL;
	    // BerrnCPU *main_cpu = NULL;
	    BerrnZ80CPU *main_cpu = NULL;

	    void writeLatch(int addr, bool line);

	    uint8_t readIO(uint16_t addr);
	    void writeIO(uint16_t addr, uint8_t data);

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x3F0> main_ram;

	    bool is_irq_enabled = false;

	    uint8_t irq_vector = 0;

	    pacmanvideo *video = NULL;

	    BerrnTimer *vblank_timer = NULL;
    };

    class LIBBLUEBERRN_API driverpacman : public berrndriver
    {
	public:
	    driverpacman();
	    ~driverpacman();

	    string drivername();
	    string parentname();
	    uint32_t get_flags();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    PacmanCore *core = NULL;
    };
};

#endif // BERRN_PACMAN