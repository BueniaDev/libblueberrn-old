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

#ifndef LIBBLUEBERRN_Z80_H
#define LIBBLUEBERRN_Z80_H

#include "z80/BeeZ80/beez80.h"
#include "scheduler.h"
using namespace beez80;
using namespace berrn;

class BerrnZ80Interface : public BeeZ80Interface
{
    public:
	BerrnZ80Interface(BerrnInterface &cb) : inter(cb)
	{

	}

	~BerrnZ80Interface()
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

	bool isSeperateOps()
	{
	    return true;
	}

	uint8_t readOpcode(uint16_t addr)
	{
	    return inter.readOp8(addr);
	}

	uint8_t portIn(uint16_t port)
	{
	    return inter.portIn(port);
	}

	void portOut(uint16_t port, uint8_t val)
	{
	    inter.portOut(port, val);
	}

    private:
	BerrnInterface &inter;
};

class BerrnZ80Processor : public BerrnProcessor
{
    public:
	BerrnZ80Processor(uint64_t clk_freq, BerrnInterface &cb) : clock_freq(clk_freq), inter(cb)
	{
	    procinter = new BerrnZ80Interface(inter);
	    core.setinterface(procinter);
	}

	~BerrnZ80Processor()
	{

	}

	void fire_nmi()
	{
	    core.generate_nmi();
	}

	void fire_interrupt8(uint8_t opcode, bool is_line = true)
	{
	    core.generate_interrupt(opcode, is_line);
	}

	void set_prescalers(int cycle_pres, int m1_pres)
	{
	    core.set_prescalers(cycle_pres, m1_pres);
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
		    cycles_left -= core.runinstruction();
		}
	    }

	    return get_exec_time();
	}

	void abort_timeslice()
	{
	    is_stopped = true;
	}

    private:
	uint64_t clock_freq = 0;
	BerrnZ80Interface *procinter = NULL;
	BerrnInterface &inter;
	BeeZ80 core;
	int64_t current_cycles = 0;
	int64_t cycles_left = 0;
	bool is_stopped = true;
};

#endif // LIBBLUEBERRN_Z80_H