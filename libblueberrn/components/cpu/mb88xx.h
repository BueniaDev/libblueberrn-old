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

#ifndef LIBBLUEBERRN_MB88XX_H
#define LIBBLUEBERRN_MB88XX_H

#include "mb88xx/mb88xx.h"
#include "scheduler.h"
using namespace mb88xx;
using namespace berrn;

class BerrnMB8843Interface : public mb88interface
{
    public:
	BerrnMB8843Interface(BerrnInterface &cb) : inter(cb)
	{

	}

	~BerrnMB8843Interface()
	{

	}

	uint8_t readROM(uint16_t addr)
	{
	    return inter.readOp8(addr);
	}

	void writeMem(uint16_t addr, uint8_t data)
	{
	    inter.writeCPU8(addr, data);
	}

    private:
	BerrnInterface &inter;
};

class BerrnMB8843Processor : public BerrnProcessor
{
    public:
	BerrnMB8843Processor(uint64_t clk_freq, BerrnInterface &cb) : clock_freq(clk_freq), inter(cb)
	{
	    procinter = new BerrnMB8843Interface(inter);
	    core.set_interface(procinter);
	}

	~BerrnMB8843Processor()
	{

	}

	void init()
	{
	    core.init();
	}

	void shutdown()
	{
	    core.shutdown();
	}

	void reset()
	{
	    core.reset();
	}

	int64_t get_exec_time()
	{
	    int64_t cycles = (current_cycles - cycles_left);
	    return static_cast<int64_t>(1e6 * cycles / clock_freq);
	}

	int64_t execute(int64_t us)
	{
	    is_stopped = false;

	    if (is_halted)
	    {
		return us;
	    }

	    current_cycles = static_cast<int64_t>(clock_freq * us / 1e6);
	    cycles_left = current_cycles;

	    while (cycles_left > 0)
	    {
		if (is_stopped)
		{
		    cycles_left = 0;
		}
		else
		{
		    int cycles = core.runinstruction();
		    cycles_left -= cycles;
		    // TODO: Implement PIO in core
		    // core.updatePIO(cycles);
		}
	    }

	    return get_exec_time();
	}

	void abort_timeslice()
	{
	    is_stopped = true;
	}

	void halt(bool is_halting)
	{
	    is_halted = is_halting;
	}

    private:
	uint64_t clock_freq = 0;
	BerrnMB8843Interface *procinter = NULL;
	BerrnInterface &inter;
	mb8843 core;
	int64_t current_cycles = 0;
	int64_t cycles_left = 0;
	bool is_stopped = true;
	bool is_halted = false;
};

#endif // LIBBLUEBERRN_MB88XX_H