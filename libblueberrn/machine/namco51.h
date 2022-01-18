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
    using n51xxinfunc = berrncbread8;
    using n51xxoutfunc = berrncbwrite8;

    class namco51xxInterface : public BerrnInterface
    {
	public:
	    namco51xxInterface(berrndriver &drv);
	    ~namco51xxInterface();

	    void init();
	    void shutdown();

	    uint8_t readOp8(uint16_t addr);
	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t portIn(uint16_t port);
	    void portOut(uint16_t port, uint8_t data);

	    void rw(bool line);
	    uint8_t read();
	    void write(uint8_t data);

	    void set_input_callback(n51xxinfunc cb)
	    {
		input_func = cb;
	    }

	    void set_output_callback(n51xxoutfunc cb)
	    {
		output_func = cb;
	    }

	    bool dump = false;

	private:
	    berrndriver &driver;
	    vector<uint8_t> mcu_rom;
	    array<uint8_t, 0x40> mcu_data;

	    n51xxinfunc input_func;
	    n51xxoutfunc output_func;

	    uint8_t portO = 0;
	    bool m_rw = false;
    };

    class namco51xx
    {
	public:
	    namco51xx(berrndriver &drv, BerrnScheduler &sched, uint64_t clk_freq);
	    ~namco51xx();

	    bool init();
	    void shutdown();

	    void set_input_callback(berrncbread8 cb);
	    void set_output_callback(berrncbwrite8 cb);

	    void set_reset_line(bool is_asserted);

	    void vblank();
	    void chip_select(bool line);
	    void rw(bool line);
	    uint8_t read();
	    void write(uint8_t data);

	    bool dump = false;

	private:
	    berrndriver &driver;
	    BerrnScheduler &scheduler;

	    namco51xxInterface *mcu_inter = NULL;

	    BerrnMB8843Processor *mcu_proc = NULL;
	    BerrnCPU *mcu_cpu = NULL;

	    BerrnTimer *write_sync = NULL;

	    bool is_reset = false;

	    array<uint8_t, 15> stack;
	    int sp = 0;
    };
};

#endif // BERRN_NAMCO51_H