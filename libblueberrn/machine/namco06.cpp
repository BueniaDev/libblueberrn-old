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

#include "namco06.h"
using namespace berrn;

namespace berrn
{
    namco06xx::namco06xx(berrndriver &drv, uint64_t clk_freq) : driver(drv), clock_freq(clk_freq)
    {
	auto &scheduler = driver.get_scheduler();

	nmi_timer = new BerrnTimer("NMI", scheduler, [&](int64_t, int64_t)
	{
	    if (next_timer_state)
	    {
		for (int i = 0; i < 4; i++)
		{
		    rw(i, testbit(control_reg, 4));
		}
	    }

	    if (next_timer_state && !read_stretch)
	    {
		set_nmi(true);
	    }
	    else
	    {
		set_nmi(false);
	    }

	    read_stretch = false;

	    for (int i = 0; i < 4; i++)
	    {
		bool is_cs_line = (testbit(control_reg, i) && next_timer_state);
		chip_select(i, is_cs_line);
	    }

	    next_timer_state = !next_timer_state;
	});

	ctrl_sync = new BerrnTimer("CtrlWrite", scheduler, [&](int64_t param, int64_t)
	{
	    uint8_t data = uint8_t(param);
	    cout << "Writing value of " << hex << int(data) << " to Namco 06xx control port" << endl;
	    writeControlSync(data);
	});

	data_sync = new BerrnTimer("DataWrite", scheduler, [&](int64_t param, int64_t)
	{
	    uint8_t data = uint8_t(param);
	    cout << "Writing value of " << hex << int(data) << " to Namco 06xx data port" << endl;
	    writeDataSync(data);
	});
    }

    namco06xx::~namco06xx()
    {

    }

    void namco06xx::set_maincpu(BerrnCPU *cpu)
    {
	nmi_cpu = cpu;
    }

    uint8_t namco06xx::readControl()
    {
	return control_reg;
    }

    void namco06xx::writeControl(uint8_t data)
    {
	ctrl_sync->start(0, false, data);
    }

    void namco06xx::writeData(uint8_t data)
    {
	data_sync->start(0, false, data);
    }

    void namco06xx::writeControlSync(uint8_t data)
    {
	control_reg = data;

	if ((control_reg & 0xE0) == 0)
	{
	    nmi_timer->stop();
	    next_timer_state = true;
	    set_nmi(false);

	    for (int i = 0; i < 4; i++)
	    {
		chip_select(i, false);
	    }
	}
	else
	{
	    if (testbit(control_reg, 4))
	    {
		set_nmi(false);
		read_stretch = true;
	    }
	    else
	    {
		read_stretch = false;
	    }

	    uint8_t num_shifts = ((control_reg >> 5) & 0x7);
	    uint8_t divisor = (1 << num_shifts);
	    int64_t period = (time_in_hz(clock_freq / divisor) / 2);

	    int64_t delay = (time_in_hz(clock_freq) / 4);

	    if (!next_timer_state)
	    {
		int64_t start_delay = (delay + time_in_hz(clock_freq / divisor));
		nmi_timer->start(start_delay, period, true);
	    }
	    else
	    {
		nmi_timer->start(delay, period, true);
	    }
	}
    }

    void namco06xx::writeDataSync(uint8_t data)
    {
	if (testbit(control_reg, 4))
	{
	    cout << "Attempted write in read mode" << endl;
	    return;
	}

	for (int i = 0; i < 4; i++)
	{
	    if (testbit(control_reg, i))
	    {
		write(i, data);
	    }
	}
    }

    void namco06xx::set_nmi(bool line)
    {
	if (nmi_cpu == NULL)
	{
	    return;
	}

	nmi_cpu->get_processor().fire_nmi(line);
    }

    void namco06xx::rw(int addr, bool line)
    {
	addr &= 3;
	if (line)
	{
	    cout << "Asserting R/W line " << dec << int(addr) << "..." << endl;
	}
	else
	{
	    cout << "Clearing R/W line " << dec << int(addr) << "..." << endl;
	}
    }

    void namco06xx::chip_select(int addr, bool line)
    {
	addr &= 3;

	if (chip_select_func)
	{
	    chip_select_func(addr, line);
	}
    }

    void namco06xx::write(int addr, uint8_t data)
    {
	addr &= 3;
	cout << "Writing value of " << hex << int(data) << " to Namco 06xx I/O device " << dec << int(addr) << endl;
    }
};