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
    namco06xx::namco06xx(BerrnCPU &cpu, uint64_t clk_freq) : main_cpu(cpu), clock_freq(clk_freq)
    {
	timer = new BerrnTimer("GenNMI", cpu.get_scheduler(), [&](int64_t, int64_t) {
	    if (rw_change && next_timer_state)
	    {
		if (!rw_stretch)
		{
		    if (rw_func)
		    {
			for (int i = 0; i < 4; i++)
			{
			    rw_func(i, testbit(control_reg, 4));
			}
		    }

		    rw_change = false;
		}
	    }

	    if (next_timer_state && !nmi_stretch)
	    {
		if (!cpu.is_suspended(SuspendReason::Reset))
		{
		    cpu.get_processor().fire_nmi();
		}
	    }

	    if (chipsel_func)
	    {
		for (int i = 0; i < 4; i++)
		{
		    bool line = (testbit(control_reg, i) && next_timer_state);
		    chipsel_func(i, line);
		}
	    }

	    next_timer_state = !next_timer_state;
	    nmi_stretch = false;
	    rw_stretch = false;
	});
    }

    namco06xx::~namco06xx()
    {

    }

    uint8_t namco06xx::read_data()
    {
	uint8_t data = 0xFF;

	if (!testbit(control_reg, 4))
	{
	    cout << "Reading in write-only mode" << endl;
	    return 0x00;
	}

	if (read_func)
	{
	    for (int i = 0; i < 4; i++)
	    {
		if (testbit(control_reg, i))
		{
		    data &= read_func(i);
		}
	    }
	}

	return data;
    }

    void namco06xx::write_data(uint8_t data)
    {
	if (testbit(control_reg, 4))
	{
	    cout << "Writing in read-only mode" << endl;
	    return;
	}

	if (write_func)
	{
	    for (int i = 0; i < 4; i++)
	    {
		if (testbit(control_reg, i))
		{
		    write_func(i, data);
		}
	    }
	}
    }

    uint8_t namco06xx::read_control()
    {
	return control_reg;
    }

    void namco06xx::write_control(uint8_t data)
    {
	control_reg = data;

	if ((control_reg & 0xE0) == 0)
	{
	    timer->stop();

	    if (chipsel_func)
	    {
		for (int i = 0; i < 4; i++)
		{
		    chipsel_func(i, false);
		}
	    }

	    next_timer_state = true;
	}
	else
	{
	    rw_stretch = !next_timer_state;
	    rw_change = true;
	    next_timer_state = true;
	    nmi_stretch = testbit(control_reg, 4);

	    uint8_t num_shifts = ((control_reg & 0xE0) >> 5);
	    uint8_t divisor = (1 << num_shifts);
	    int64_t timer_period = (time_in_hz(clock_freq / divisor) / 2);
	    timer->start(time_zero(), timer_period, true);
	}
    }
};