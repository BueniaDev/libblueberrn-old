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

#ifndef LIBBLUEBERRN_8080_H
#define LIBBLUEBERRN_8080_H

#include "8080/Bee8080/bee8080.h"
#include "scheduler.h"
using namespace bee8080;
using namespace berrn;

class Berrn8080Interface : public Bee8080Interface
{
    public:
	Berrn8080Interface(BerrnInterface &cb) : inter(cb)
	{

	}

	~Berrn8080Interface()
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

	uint8_t portIn(uint8_t port)
	{
	    return inter.portIn(port);
	}

	void portOut(uint8_t port, uint8_t val)
	{
	    inter.portOut(port, val);
	}

    private:
	BerrnInterface &inter;
};

class Berrn8080Processor : public BerrnProcessor
{
    public:
	Berrn8080Processor(uint64_t clk_freq, BerrnInterface &cb) : clock_freq(clk_freq), inter(cb)
	{
	    procinter = new Berrn8080Interface(inter);
	    core.setinterface(procinter);
	}

	~Berrn8080Processor()
	{

	}

	void fire_interrupt8(uint8_t opcode = 0xFF, bool is_line = true)
	{
	    if (is_line)
	    {
		core.setinterrupt(opcode);
	    }
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
	    return static_cast<int64_t>((1e6 * cycles / clock_freq) + 0.5);
	}

	int64_t execute(int64_t us)
	{
	    is_stopped = false;
	    current_cycles = static_cast<int64_t>(clock_freq * us / 1e6);
	    cycles_left = current_cycles;

	    do
	    {
		if (is_stopped)
		{
		    cycles_left = 0;
		}
		else
		{
		    cycles_left -= core.runinstruction();
		}
	    }
	    while (cycles_left > 0);

	    return get_exec_time();
	}

	void abort_timeslice()
	{
	    is_stopped = true;
	}

    private:
	Berrn8080Interface *procinter = NULL;
	uint64_t clock_freq = 0;
	BerrnInterface &inter;
	Bee8080 core;
	int64_t current_cycles = 0;
	int64_t cycles_left = 0;
	bool is_stopped = true;
};

class Berrn8080CPU : public BerrnCPU
{
    public:
	Berrn8080CPU(berrndriver &drv, uint64_t clk_freq, BerrnInterface &cb) : 
	    BerrnCPU(drv.get_scheduler(), new Berrn8080Processor(clk_freq, cb))
	{

	}

	void init()
	{
	    get_processor().init();
	}

	void shutdown()
	{
	    get_processor().shutdown();
	}

	void reset()
	{
	    get_processor().reset();
	}

	void fireInterrupt8(uint8_t opcode = 0xFF, bool is_line = true)
	{
	    get_processor().fire_interrupt8(opcode, is_line);
	}

	void clearInterrupt()
	{
	    get_processor().clear_interrupt();
	}

	void debugOutput()
	{
	    get_processor().debug_output();
	}
};

#endif // LIBBLUEBERRN_8080_H