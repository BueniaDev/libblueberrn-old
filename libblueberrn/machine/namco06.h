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
#include <driver.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    using csfunc = function<void(int, bool)>;

    class namco06xx
    {
	public:
	    namco06xx(berrndriver &drv, uint64_t clk_freq);
	    ~namco06xx();

	    void set_maincpu(BerrnCPU *cpu);

	    uint8_t readControl();
	    void writeControl(uint8_t data);

	    void writeData(uint8_t data);

	    void set_chip_select(csfunc cb)
	    {
		chip_select_func = cb;
	    }

	private:
	    berrndriver &driver;
	    uint64_t clock_freq = 0;

	    csfunc chip_select_func;

	    BerrnCPU *nmi_cpu = NULL;

	    BerrnTimer *ctrl_sync = NULL;
	    BerrnTimer *data_sync = NULL;
	    BerrnTimer *nmi_timer = NULL;

	    void writeControlSync(uint8_t data);
	    void writeDataSync(uint8_t data);

	    uint8_t control_reg = 0;

	    void set_nmi(bool line);

	    void rw(int addr, bool line);
	    void chip_select(int addr, bool line);

	    void write(int addr, uint8_t data);

	    bool next_timer_state = false;
	    bool read_stretch = false;
    };
};


#endif // BERRN_NAMCO06_H