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