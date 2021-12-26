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

#ifndef BERRN_NAMCO51_H
#define BERRN_NAMCO51_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <driver.h>
#include <scheduler.h>
#include <cpu/mb88xx.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class namco51xxInterface : public BerrnInterface
    {
	public:
	    namco51xxInterface();
	    ~namco51xxInterface();

	    void set_rom(vector<uint8_t> rom_vec)
	    {
		copy(rom_vec.begin(), (rom_vec.begin() + 0x400), rom_file.begin());
	    }

	    uint8_t readOp8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    array<uint8_t, 0x400> rom_file;
	    array<uint8_t, 0x40> rom_data;
    };

    class namco51xx
    {
	public:
	    namco51xx(berrndriver &drv, BerrnScheduler &sched, uint32_t clock_freq);
	    ~namco51xx();

	    void init();
	    void shutdown();

	    void set_reset_line(bool is_asserted);

	private:
	    berrndriver &driver;
	    BerrnScheduler &scheduler;

	    namco51xxInterface *mcu_inter = NULL;
	    BerrnMB8843Processor *mcu_proc = NULL;
	    BerrnCPU *mcu_cpu = NULL;
    };
};

#endif // BERRN_NAMCO51_H