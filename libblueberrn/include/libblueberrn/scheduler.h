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

#ifndef BERRNSCHEDULER_H
#define BERRNSCHEDULER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cassert>
#include <vector>
#include <array>
#include <functional>
#include <algorithm>
#include <utils.h>
using namespace std;

namespace berrn
{
    inline int64_t time_zero()
    {
	return 0;
    }

    inline int64_t time_in_hz(double hz)
    {
	return int64_t(1e6 / hz);
    }

    inline int64_t time_in_usec(int usec)
    {
	return usec;
    }

    inline int64_t time_in_sec(int sec)
    {
	return (sec * 1e6);
    }

    inline int64_t time_in_msec(int msec)
    {
	return (msec * 1000);
    }

    class BerrnScheduler;
    class BerrnTimer;
    using timerfunc = function<void(int64_t, int64_t)>;

    class BerrnDevice
    {
	public:
	    virtual void reset() = 0;
	    virtual int64_t get_exec_time() = 0;
	    virtual int64_t execute(int64_t until_time) = 0; // usecs
	    virtual void stop_timeslice() = 0;
    };

    class BerrnNull : public BerrnDevice
    {
	public:
	    void reset()
	    {
		usec_time = 0;
	    }

	    int64_t get_exec_time()
	    {
		return usec_diff;
	    }

	    int64_t execute(int64_t until_time)
	    {
		usec_diff = (until_time - usec_time);
		usec_time = until_time;
		return until_time;
	    }

	    void stop_timeslice()
	    {
		usec_time = 0;
	    }

	private:
	    int64_t usec_time = 0;
	    int64_t usec_diff = 0;
    };

    class BerrnProcessor
    {
	public:
	    ~BerrnProcessor()
	    {

	    }

	    virtual void fire_nmi()
	    {
		return;
	    }

	    virtual void fire_interrupt(bool is_line = true)
	    {
		(void)is_line;
		return;
	    }

	    virtual void fire_interrupt8(uint8_t opcode = 0xFF, bool is_line = true)
	    {
		(void)opcode;
		(void)is_line;
		return;
	    }

	    virtual void fire_interrupt_level(int level, bool is_line = true)
	    {
		(void)level;
		(void)is_line;
		return;
	    }

	    virtual void init()
	    {
		return;
	    }

	    virtual void shutdown()
	    {
		return;
	    }

	    virtual void reset()
	    {
		return;
	    }

	    virtual int64_t get_exec_time()
	    {
		return 0;
	    }

	    virtual int64_t execute(int64_t until_time) // usecs
	    {
		(void)until_time;
		return 0;
	    }

	    virtual void abort_timeslice()
	    {
		return;
	    }

	    virtual void halt(bool is_halting)
	    {
		(void)is_halting;
		return;
	    }

	    virtual void debug_output()
	    {
		return;
	    }
    };

    class BerrnInterface
    {
	public:
	    virtual uint8_t readCPU8(uint16_t addr)
	    {
		cout << "Reading byte from address of " << hex << (int)addr << endl;
		exit(0);
		return 0;
	    }

	    virtual void writeCPU8(uint16_t addr, uint8_t val)
	    {
		cout << "Writing byte of " << hex << (int)val << " to address of " << hex << (int)addr << endl;
		exit(0);
	    }

	    virtual uint8_t readOp8(uint16_t addr)
	    {
		cout << "Reading opcode from address of " << hex << (int)addr << endl;
		exit(0);
		return 0;
	    }

	    virtual uint16_t readCPU16(bool upper, bool lower, uint32_t addr)
	    {
		cout << "Reading word from below address:" << endl;
		cout << "Upper: " << dec << int(upper) << endl;
		cout << "Lower: " << dec << int(lower) << endl;
		cout << "Address: " << hex << int(addr) << endl;
		exit(0);
		return 0;
	    }

	    virtual bool isSeperateOps()
	    {
		return false;
	    }

	    virtual void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t val)
	    {
		cout << "Writing word to below address:" << endl;
		cout << "Upper: " << dec << int(upper) << endl;
		cout << "Lower: " << dec << int(lower) << endl;
		cout << "Address: " << hex << int(addr) << endl;
		cout << "Data: " << hex << int(val) << endl;
		exit(0);
	    }

	    virtual uint8_t portIn(uint16_t port)
	    {
		cout << "Reading byte from port of " << hex << (int)(port & 0xFF) << endl;
		exit(0);
		return 0;
	    }

	    virtual void portOut(uint16_t port, uint8_t val)
	    {
		cout << "Writing byte of " << hex << (int)val << " to port of " << hex << (int)(port & 0xFF) << endl;
		exit(0);
	    }
    };

    class BerrnScheduler
    {
	public:
	    BerrnScheduler()
	    {
		set_quantum(time_in_hz(60));
		current_time = 0;
	    }

	    ~BerrnScheduler()
	    {

	    }

	    void set_quantum(int64_t quantum_val)
	    {
		quantum_time = quantum_val;
	    }

	    void reset()
	    {
		current_time = 0;
	    }

	    void shutdown()
	    {
		for (auto &timer : timers)
		{
		    remove_timer(timer);
		}

		for (auto &device : devices)
		{
		    remove_device(device);
		}
	    }

	    void add_timer(BerrnTimer *timer);

	    void remove_timer(BerrnTimer *timer)
	    {
		for (auto it = timers.begin(); it != timers.end(); it++)
		{
		    if (*it == timer)
		    {
			timers.erase(it);
			return;
		    }
		}
	    }

	    void execute_timers();

	    void add_device(BerrnDevice *device)
	    {
		devices.push_back(device);
	    }

	    void remove_device(BerrnDevice *device)
	    {
		for (auto it = devices.begin(); it != devices.end(); it++)
		{
		    if (*it == device)
		    {
			devices.erase(it);
			break;
		    }
		}
	    }

	    void timeslice()
	    {
		execute_timers();
		int64_t until_time = (current_time + quantum_time);

		for (auto &dev : devices)
		{
		    int64_t timer_val = get_time_next_timer();

		    if ((timer_val > 0) && (timer_val < until_time))
		    {
			until_time = timer_val;
		    }

		    if (until_time > current_time)
		    {
			current_dev = dev;
			int64_t exec_time = dev->execute(until_time);

			if (exec_time < until_time)
			{
			    until_time = exec_time;
			}

			current_dev = NULL;
		    }
		}

		current_time = until_time;
	    }

	    int64_t get_current_time()
	    {
		int64_t result = current_time;

		if (current_dev != NULL)
		{
		    result += current_dev->get_exec_time();
		}

		return result;
	    }

	    int64_t get_time_next_timer();

	private:
	    int64_t quantum_time = 0;
	    int64_t current_time = 0;

	    vector<BerrnTimer*> timers;
	    vector<BerrnDevice*> devices;
	    BerrnDevice *current_dev = NULL;
    };

    class BerrnTimer
    {
	public:
	    BerrnTimer(string tag, BerrnScheduler &sched, timerfunc cb) : tag_str(tag), scheduler(sched), timercallback(cb)
	    {
		expiration_time = 0;
		is_enabled = false;
		period_usec = 0;
		is_periodic = false;
	    }

	    ~BerrnTimer()
	    {

	    }

	    void start(int64_t period, bool is_persistent, int64_t param = 0)
	    {
		start(period, period, is_persistent, param);
	    }

	    void start(int64_t start_delay, int64_t period, bool is_persistent, int64_t param = 0)
	    {
		assert(period >= 0);
		timer_param = param;
		period_usec = period;
		is_periodic = is_persistent;
		is_enabled = true;
		scheduler.remove_timer(this);
		expiration_time = (scheduler.get_current_time() + start_delay);
		scheduler.add_timer(this);
	    }

	    void stop()
	    {
		is_enabled = false;
	    }

	    void ontick(int64_t current_time)
	    {
		if (is_enabled && (expiration_time <= current_time))
		{
		    if (is_periodic)
		    {
			scheduler.remove_timer(this);

			while (expiration_time <= current_time)
			{
			    expiration_time += period_usec;
			}

			scheduler.add_timer(this);
		    }
		    else
		    {
			is_enabled = false;
		    }

		    timercallback(timer_param, current_time);
		}
	    }

	    int64_t expiration_time;
	    bool is_enabled;
	    string tag_str;

	private:
	    BerrnScheduler &scheduler;
	    timerfunc timercallback;
	    int64_t period_usec;
	    bool is_periodic;
	    int64_t timer_param;
    };

    enum SuspendReason : int
    {
	Reset = 0,
	AnyReason = 6,
    };

    class BerrnCPU : public BerrnDevice
    {
	public:
	    BerrnCPU(BerrnScheduler &sched, BerrnProcessor &proc) : scheduler(sched), processor(proc)
	    {
		current_time = 0;
	    }

	    virtual ~BerrnCPU()
	    {

	    }

	    virtual void reset()
	    {
		processor.reset();
	    }

	    virtual int64_t get_exec_time()
	    {
		return processor.get_exec_time();
	    }

	    virtual int64_t execute(int64_t until_time) // useconds
	    {
		if (until_time > current_time)
		{
		    current_time += processor.execute((until_time - current_time));
		}
		else
		{
		    current_time = until_time;
		}

		return current_time;
	    }

	    bool is_suspended(SuspendReason reason = SuspendReason::AnyReason)
	    {
		if (reason == SuspendReason::AnyReason)
		{
		    return (suspend_flags != 0);
		}

		return testbit(suspend_flags, int(reason));
	    }

	    void suspend(SuspendReason reason)
	    {
		if (reason == SuspendReason::AnyReason)
		{
		    return;
		}

		suspend_flags = setbit(suspend_flags, int(reason));
		processor.abort_timeslice();
		processor.halt(true);
	    }

	    void resume(SuspendReason reason)
	    {
		if (reason == SuspendReason::AnyReason)
		{
		    return;
		}

		suspend_flags = resetbit(suspend_flags, int(reason));
		processor.abort_timeslice();
		processor.halt(false);
	    }

	    void set_reset_line(bool is_asserted)
	    {
		if (is_asserted)
		{
		    suspend(SuspendReason::Reset);
		}
		else
		{
		    if (is_suspended(SuspendReason::Reset))
		    {
			reset();
			resume(SuspendReason::Reset);
		    }
		}
	    }

	    void stop_timeslice()
	    {
		processor.abort_timeslice();
	    }

	    BerrnProcessor& get_processor()
	    {
		return processor;
	    }

	    BerrnScheduler& get_scheduler()
	    {
		return scheduler;
	    }

	private:
	    BerrnScheduler &scheduler;
	    BerrnProcessor &processor;

	    int64_t current_time = 0;
	    uint32_t suspend_flags = 0;
    };
};

#endif // BERRNSCHEDULER_H