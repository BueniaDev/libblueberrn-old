/*
    This file is part of libblueberrn.
    Copyright (C) 2021 BueniaDev.

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

#include "namco51.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(namco51)
      berrn_rom_region("mcu", 0x400)
        berrn_rom_load("51xx.bin", 0x0000, 0x0400)
    berrn_rom_end

    namco51xxInterface::namco51xxInterface()
    {
	rom_data.fill(0);
    }

    namco51xxInterface::~namco51xxInterface()
    {

    }

    void namco51xxInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x40)
	{
	    rom_data.at(addr) = data;
	}
    }

    uint8_t namco51xxInterface::readOp8(uint16_t addr)
    {
	uint8_t temp = 0x00;
	if (addr < 0x400)
	{
	    temp = rom_file.at(addr);
	}

	return temp;
    }

    namco51xx::namco51xx(berrndriver &drv, BerrnScheduler &sched, uint32_t clock_freq) : driver(drv), scheduler(sched)
    {
	mcu_inter = new namco51xxInterface();
	mcu_proc = new BerrnMB8843Processor((clock_freq / 12), *mcu_inter);
	mcu_cpu = new BerrnCPU(scheduler, *mcu_proc);
    }

    namco51xx::~namco51xx()
    {

    }

    void namco51xx::init()
    {
	driver.loadROM(berrn_rom_name(namco51));
	auto rom_file = driver.get_rom_region("mcu");

	cout << "Namco 51xx ROM size: " << hex << int(rom_file.size()) << endl;
	mcu_inter->set_rom(rom_file);
	scheduler.add_device(mcu_cpu);
	mcu_proc->init();
    }

    void namco51xx::shutdown()
    {
	scheduler.remove_device(mcu_cpu);
	mcu_proc->shutdown();
    }

    void namco51xx::set_reset_line(bool is_asserted)
    {
	if (is_asserted)
	{
	    mcu_cpu->suspend(SuspendReason::Reset);
	}
	else
	{
	    if (mcu_cpu->is_suspended(SuspendReason::Reset))
	    {
		mcu_cpu->reset();
		mcu_cpu->resume(SuspendReason::Reset);
	    }
	}
    }
};