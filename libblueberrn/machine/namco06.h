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

#ifndef BERRN_NAMCO06_H
#define BERRN_NAMCO06_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <scheduler.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    using namco06read = function<uint8_t(int)>;
    using namco06write = function<void(int, uint8_t)>;
    using namco06rw = function<void(int, bool)>;
    using namco06sel = function<void(int, bool)>;

    class namco06xx
    {
	public:
	    namco06xx(BerrnCPU &cpu, uint64_t clk_freq);
	    ~namco06xx();

	    uint8_t read_control();
	    uint8_t read_data();

	    void write_control(uint8_t data);
	    void write_data(uint8_t data);

	    void set_read_callback(namco06read func)
	    {
		read_func = func;
	    }

	    void set_write_callback(namco06write func)
	    {
		write_func = func;
	    }

	    void set_chipsel_callback(namco06sel func)
	    {
		chipsel_func = func;
	    }

	    void set_rw_callback(namco06rw func)
	    {
		rw_func = func;
	    }

	private:
	    BerrnTimer *timer = NULL;
	    BerrnCPU &main_cpu;

	    uint8_t control_reg = 0;

	    uint64_t clock_freq;

	    namco06read read_func;
	    namco06write write_func;

	    namco06rw rw_func;
	    namco06sel chipsel_func;

	    bool rw_stretch = false;
	    bool rw_change = false;
	    bool next_timer_state = false;
	    bool nmi_stretch = false;
    };
};


#endif // BERRN_NAMCO06_H