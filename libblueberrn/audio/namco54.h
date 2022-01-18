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

#ifndef BERRN_NAMCO54_H
#define BERRN_NAMCO54_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <driver.h>
#include <scheduler.h>
#include <cpu/mb88xx.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class namco54xxInterface : public BerrnInterface
    {
	public:
	    namco54xxInterface(berrndriver &drv);
	    ~namco54xxInterface();

	    void init();
	    void shutdown();

	    uint8_t readOp8(uint16_t addr);
	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t portIn(uint16_t addr);
	    void portOut(uint16_t addr, uint8_t data);

	    void write(uint8_t data);

	private:
	    berrndriver &driver;
	    vector<uint8_t> mcu_rom;
	    array<uint8_t, 0x40> mcu_data;

	    uint8_t latched_cmd = 0;
    };

    class namco54xx
    {
	public:
	    namco54xx(berrndriver &drv, BerrnScheduler &sched, uint64_t clk_freq);
	    ~namco54xx();

	    bool init();
	    void shutdown();

	    void chip_select(bool line);
	    void write(uint8_t data);

	    void set_reset_line(bool is_asserted);

	    void set_irq_duration(int64_t duration)
	    {
		irq_duration = duration;
	    }

	private:
	    berrndriver &driver;
	    BerrnScheduler &scheduler;

	    namco54xxInterface *mcu_inter = NULL;

	    int64_t irq_duration = 0;

	    BerrnTimer *set_latched_timer = NULL;
	    BerrnTimer *irq_timer = NULL;

	    BerrnMB8844Processor *mcu_proc = NULL;
	    BerrnCPU *mcu_cpu = NULL;

	    bool is_reset = false;
    };
};

#endif // BERRN_NAMCO54_H