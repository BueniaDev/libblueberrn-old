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
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API PacmanInterface : public BerrnInterface
    {
	public:
	    PacmanInterface();
	    ~PacmanInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t readOp8(uint16_t addr);
	    void portOut(uint16_t port, uint8_t val);
	    void updatePixels();

	    vector<uint8_t> &get_gamerom()
	    {
		return gamerom;
	    }

	    bool is_vblank_enabled()
	    {
		return vblank_enable;
	    }

	    uint8_t get_int_vec()
	    {
		return int_vector;
	    }

	private:
	    uint8_t readByte(uint16_t addr);

	    void writeIO(int addr, uint8_t data);

	    vector<uint8_t> gamerom;
	    array<uint8_t, 0x400> vram;
	    array<uint8_t, 0x400> cram;
	    array<uint8_t, 0x3F0> mainram;
	    array<uint8_t, 0x10> oam;
	    array<uint8_t, 0x10> sprite_pos;
	    bool vblank_enable = false;
	    bool sound_enable = false;
	    bool flip_screen = false;

	    uint8_t int_vector = 0;
    };

    class LIBBLUEBERRN_API driverpacman : public berrndriver
    {
	public:
	    driverpacman();
	    ~driverpacman();

	    string drivername();
	    bool hasdriverROMs();

	    virtual void loadROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    PacmanInterface inter;
	    BerrnScheduler scheduler;

	    void interrupt_handler();

	    BerrnZ80Processor *pacman_proc = NULL;
	    BerrnCPU *pacman_cpu = NULL;

	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *vblank_timer = NULL;
    };
};


#endif // BERRN_PACMAN