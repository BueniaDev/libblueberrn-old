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
using namespace std;

namespace berrn
{
    inline int64_t time_in_hz(int64_t hz)
    {
	return (1e6 / hz);
    }

    inline int64_t time_in_usec(int64_t usec)
    {
	return usec;
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

    class BerrnProcessor
    {
	public:
	    virtual ~BerrnProcessor()
	    {

	    }

	    virtual void fire_nmi()
	    {
		return;
	    }

	    virtual void fire_interrupt8(uint8_t opcode, bool is_line = true)
	    {
		(void)opcode;
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
    };

    class BerrnInterface
    {
	public:
	    virtual void init()
	    {
		return;
	    }

	    virtual void shutdown()
	    {
		return;
	    }

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
		current_fps = 60;
		current_interleave = 1;
		update_quantum();
		current_time = 0;
	    }

	    ~BerrnScheduler()
	    {

	    }

	    void set_fps(int64_t fps)
	    {
		current_fps = fps;
		update_quantum();
	    }

	    void set_interleave(int64_t interleave)
	    {
		current_interleave = interleave;
		update_quantum();
	    }

	    void reset()
	    {
		current_time = 0;
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

		current_time = static_cast<uint64_t>(until_time);
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
	    void update_quantum()
	    {
		quantum_time = ((1e6 / current_fps) / current_interleave);
	    }

	    int64_t current_fps = 0;
	    int64_t current_interleave = 0;

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

		scheduler.add_timer(this);
	    }

	    ~BerrnTimer()
	    {

	    }

	    void start(int64_t period, bool is_persistent, int64_t param = 0)
	    {
		assert(period >= 0);
		timer_param = param;
		period_usec = period;
		is_periodic = is_persistent;
		is_enabled = true;
		scheduler.remove_timer(this);
		expiration_time = (scheduler.get_current_time() + period);
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

	private:
	    string tag_str;
	    BerrnScheduler &scheduler;
	    timerfunc timercallback;
	    int64_t period_usec;
	    bool is_periodic;
	    int64_t timer_param;
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
    };
};

#endif // BERRNSCHEDULER_H