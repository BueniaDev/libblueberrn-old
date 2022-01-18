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

#include "scheduler.h"
using namespace std;

namespace berrn
{
    void BerrnScheduler::add_timer(BerrnTimer *timer)
    {
	bool stop_timeslice = (timer->expiration_time < get_time_next_timer());
	timers.push_back(timer);

	if (stop_timeslice)
	{
	    if (current_dev != NULL)
	    {
		current_dev->stop_timeslice();
	    }
	}
    }

    void BerrnScheduler::execute_timers()
    {
	for (auto &timer : timers)
	{
	    timer->ontick(current_time);
	}
    }

    int64_t BerrnScheduler::get_time_next_timer()
    {
	int64_t time_val = INT64_MAX;

	for (auto &timer : timers)
	{
	    if (timer->is_enabled && (timer->expiration_time < time_val))
	    {
		time_val = timer->expiration_time;
	    }
	}

	return (time_val == INT64_MAX) ? 0 : time_val;
    }
}