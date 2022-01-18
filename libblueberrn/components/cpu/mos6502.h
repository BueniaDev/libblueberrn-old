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

#ifndef LIBBLUEBERRN_M6502_H
#define LIBBLUEBERRN_M6502_H

#include "m6502/Bee6502/Bee6502/bee6502.h"
#include "scheduler.h"
using namespace bee6502;
using namespace berrn;

class Berrn6502Interface : public Bee6502Interface
{
    public:
	Berrn6502Interface(BerrnInterface &cb) : inter(cb)
	{

	}

	~Berrn6502Interface()
	{

	}

	uint8_t readByte(uint16_t addr)
	{
	    return inter.readCPU8(addr);
	}

	void writeByte(uint16_t addr, uint8_t val)
	{
	    inter.writeCPU8(addr, val);
	}

    private:
	BerrnInterface &inter;
};

class Berrn6502Processor : public BerrnProcessor
{
    public:
	Berrn6502Processor(uint64_t clk_freq, BerrnInterface &cb) : clock_freq(clk_freq), inter(cb)
	{
	    procinter = new Berrn6502Interface(inter);
	    core.setinterface(procinter);
	    core.setBCD(true);
	}

	~Berrn6502Processor()
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
	    // TODO: Implement reset function in Bee6502
	    core.init();
	}

	int64_t get_exec_time()
	{
	    int64_t cycles = (current_cycles - cycles_left);
	    return static_cast<int64_t>((1e6 * cycles / clock_freq) + 0.5);
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
		    debug_output();
		    cycles_left -= core.runinstruction();
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

	void debug_output()
	{
	    cout << "M6502 output: " << endl;
	    core.debugoutput();
	    cout << endl;
	}

    private:
	uint64_t clock_freq = 0;
	Berrn6502Interface *procinter = NULL;
	BerrnInterface &inter;
	Bee6502 core;
	int64_t current_cycles = 0;
	int64_t cycles_left = 0;
	bool is_stopped = true;
	bool is_halted = false;
	bool dump = false;

	bool is_irq_line = false;
};

#endif // LIBBLUEBERRN_M6502_H