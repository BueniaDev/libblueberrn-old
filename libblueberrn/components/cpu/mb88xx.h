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

#ifndef LIBBLUEBERRN_MB88XX_H
#define LIBBLUEBERRN_MB88XX_H

#include "mb88xx/mb88xx.h"
#include "scheduler.h"
using namespace mb88xx;
using namespace berrn;

class BerrnMB88XXInterface : public mb88xxInterface
{
    public:
	BerrnMB88XXInterface(BerrnInterface &cb) : inter(cb)
	{

	}

	~BerrnMB88XXInterface()
	{

	}

	uint8_t readROM(uint16_t addr)
	{
	    return inter.readOp8(addr);
	}

	uint8_t readMem(uint16_t addr)
	{
	    return inter.readCPU8(addr);
	}

	void writeMem(uint16_t addr, uint8_t data)
	{
	    inter.writeCPU8(addr, data);
	}

	uint8_t readR(int addr)
	{
	    addr &= 3;
	    return inter.portIn(addr);
	}

	void writeR(int addr, uint8_t data)
	{
	    addr &= 3;
	    inter.portOut(addr, data);
	}

	uint8_t readK()
	{
	    return inter.portIn(4);
	}

	void writeO(uint8_t data)
	{
	    inter.portOut(4, data);
	}

	void writeP(uint8_t data)
	{
	    inter.portOut(5, data);
	}

    private:
	BerrnInterface &inter;
};

template<class T>
class BerrnMB88XXProcessor : public BerrnProcessor
{
    public:
	BerrnMB88XXProcessor(uint64_t clk_freq, BerrnInterface &cb) : clock_freq(clk_freq), inter(cb)
	{
	    procinter = new BerrnMB88XXInterface(inter);
	    core.setinterface(procinter);
	}

	~BerrnMB88XXProcessor()
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

	void clock_write(bool line)
	{
	    core.clock_write(line);
	}

	void fire_irq(bool line)
	{
	    core.fire_irq(line);
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
	    cout << "MB88XX output:" << endl;
	    core.debugoutput();
	    cout << endl;
	}

	void set_dump(bool val)
	{
	    core.dump = true;
	}

	bool dump = false;

    private:
	uint64_t clock_freq = 0;
	BerrnMB88XXInterface *procinter = NULL;
	BerrnInterface &inter;
	T core;
	int64_t current_cycles = 0;
	int64_t cycles_left = 0;
	bool is_stopped = true;
	bool is_halted = false;
};

using BerrnMB8843Processor = BerrnMB88XXProcessor<mb8843core>;
using BerrnMB8844Processor = BerrnMB88XXProcessor<mb8844core>;

#endif // LIBBLUEBERRN_MB88XX_H