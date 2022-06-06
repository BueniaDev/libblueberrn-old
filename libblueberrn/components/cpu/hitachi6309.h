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

#ifndef LIBBLUEBERRN_HD6309_H
#define LIBBLUEBERRN_HD6309_H

#include "m680x/Botnami/Botnami/botnami.h"
#include "scheduler.h"
using namespace botnami;
using namespace berrn;

class BerrnHD6309Interface : public BotnamiInterface
{
    public:
	BerrnHD6309Interface(BerrnInterface &cb) : inter(cb)
	{

	}

	~BerrnHD6309Interface()
	{

	}

	uint8_t readByte(uint16_t addr)
	{
	    return inter.readCPU8(addr);
	}

	bool isSeperateOps()
	{
	    return inter.isSeperateOps();
	}

	uint8_t readOpcode(uint16_t addr)
	{
	    return inter.readOp8(addr);
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    inter.writeCPU8(addr, data);
	}

    private:
	BerrnInterface &inter;
};

class BerrnHD6309Processor : public BerrnProcessor
{
    public:
	BerrnHD6309Processor(uint64_t clk_freq, BerrnInterface &cb) : clock_freq(clk_freq), inter(cb)
	{
	    procinter = new BerrnHD6309Interface(inter);
	    core.setinterface(*procinter);
	}

	~BerrnHD6309Processor()
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

	    do
	    {
		if (is_stopped)
		{
		    cycles_left = 0;
		}
		else
		{
		    // core.debugoutput();
		    cycles_left -= core.executenextinstr();
		}
	    }
	    while (cycles_left > 0);

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
	    cout << "HD6309 output: " << endl;
	    core.debugoutput();
	}

    private:
	uint64_t clock_freq = 0;
	BerrnHD6309Interface *procinter = NULL;
	BerrnInterface &inter;
	Botnami6309 core;
	int64_t current_cycles = 0;
	int64_t cycles_left = 0;
	bool is_stopped = true;
	bool is_halted = false;
	bool dump = false;
};

#endif // LIBBLUEBERRN_HD6309_H