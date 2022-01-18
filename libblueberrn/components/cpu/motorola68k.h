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

#ifndef LIBBLUEBERRN_M68K_H
#define LIBBLUEBERRN_M68K_H

#include "m68k/Botsashi/Botsashi/botsashi.h"
#include "scheduler.h"
using namespace botsashi;
using namespace berrn;

class BerrnM68KInterface : public BotsashiInterface
{
    public:
	BerrnM68KInterface(BerrnInterface &cb) : inter(cb)
	{

	}

	~BerrnM68KInterface()
	{

	}

	uint16_t readWord(bool upper, bool lower, uint32_t addr)
	{
	    return inter.readCPU16(upper, lower, addr);
	}

	void writeWord(bool upper, bool lower, uint32_t addr, uint16_t val)
	{
	    inter.writeCPU16(upper, lower, addr, val);
	}

	bool istrapOverride(int val)
	{
	    (void)val;
	    return false;
	}

	void trapException(int val, Botsashi &m68k)
	{
	    (void)val;
	    (void)m68k;
	    return;
	}

	void stopFunction()
	{
	    return;
	}

    private:
	BerrnInterface &inter;
};

class BerrnM68KProcessor : public BerrnProcessor
{
    public:
	BerrnM68KProcessor(uint64_t clk_freq, BerrnInterface &cb) : clock_freq(clk_freq), inter(cb)
	{
	    procinter = new BerrnM68KInterface(inter);
	    core.setinterface(*procinter);
	}

	~BerrnM68KProcessor()
	{

	}

	void init()
	{
	    core.init();
	    core.reset_exception();
	}

	void shutdown()
	{
	    core.shutdown();
	}

	void reset()
	{
	    core.reset_exception();
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
		    // core.debugoutput();
		    int cycles = core.executenextinstr();

		    /*
		    if (cycles == 0)
		    {
			cout << "Instruction has unimplemented cycle timings" << endl;
			exit(0);
		    }
		    */
		    cycles_left -= cycles;
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
	BerrnM68KInterface *procinter = NULL;
	BerrnInterface &inter;
	Botsashi core;
	int64_t current_cycles = 0;
	int64_t cycles_left = 0;
	bool is_stopped = true;
	bool is_halted = false;
};

#endif // LIBBLUEBERRN_M68K_H