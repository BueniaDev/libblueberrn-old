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

#include "namco54.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(namco54)
      berrn_rom_region("mcu54", 0x400, 0)
        berrn_rom_load("54xx.bin", 0x0000, 0x0400)
    berrn_rom_end

    namco54xxInterface::namco54xxInterface(berrndriver &drv) : driver(drv)
    {

    }

    namco54xxInterface::~namco54xxInterface()
    {

    }

    void namco54xxInterface::init()
    {
	mcu_rom = driver.get_rom_region("mcu54");
	mcu_data.fill(0);
    }

    void namco54xxInterface::shutdown()
    {
	mcu_rom.clear();
    }

    uint8_t namco54xxInterface::readOp8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x400)
	{
	    data = mcu_rom.at(addr);
	}

	return data;
    }

    uint8_t namco54xxInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x40)
	{
	    data = mcu_data.at(addr);
	}

	return data;
    }

    void namco54xxInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x40)
	{
	    mcu_data.at(addr) = data;
	}
    }

    uint8_t namco54xxInterface::portIn(uint16_t addr)
    {
	uint8_t data = 0xFF;
	switch (addr)
	{
	    case 0: data = (latched_cmd & 0xF); break;
	    case 4: data = (latched_cmd >> 4); break;
	    default: break;
	}

	return data;
    }

    void namco54xxInterface::portOut(uint16_t addr, uint8_t data)
    {
	switch (addr)
	{
	    case 1:
	    {
		cout << "Writing value of " << hex << int(data & 0xF) << " to Namco 54xx discrete input 2" << endl;
	    }
	    break;
	    case 4:
	    {
		uint8_t out = (data & 0xF);
		int input_num = testbit(data, 4) ? 1 : 0;
		cout << "Writing value of " << hex << int(out) << " to Namco 54xx discrete input " << dec << input_num << endl;
	    }
	    break;
	    default: break;
	}
    }

    void namco54xxInterface::write(uint8_t data)
    {
	latched_cmd = data;
    }

    namco54xx::namco54xx(berrndriver &drv, BerrnScheduler &sched, uint64_t clk_freq) : driver(drv), scheduler(sched), irq_duration(time_in_usec(100))
    {
	mcu_inter = new namco54xxInterface(driver);
	mcu_proc = new BerrnMB8844Processor(clk_freq, *mcu_inter);
	mcu_cpu = new BerrnCPU(scheduler, *mcu_proc);

	irq_timer = new BerrnTimer("IRQ Timer", scheduler, [&](int64_t, int64_t)
	{
	    mcu_proc->fire_irq(false);
	});

	set_latched_timer = new BerrnTimer("SetLatched", scheduler, [&](int64_t param, int64_t)
	{
	    uint8_t data = uint8_t(param);
	    mcu_inter->write(data);
	});
    }

    namco54xx::~namco54xx()
    {

    }

    bool namco54xx::init()
    {
	if (!driver.loadROM(berrn_rom_name(namco54)))
	{
	    return false;
	}

	mcu_inter->init();
	mcu_proc->init();
	scheduler.add_device(mcu_cpu);
	return true;
    }

    void namco54xx::shutdown()
    {
	mcu_inter->shutdown();
	irq_timer->stop();
	scheduler.remove_device(mcu_cpu);
    }

    void namco54xx::chip_select(bool line)
    {
	// TODO: Broken sound occurs if we use this 
	// mcu_proc->fire_irq(line);
	(void)line;
	return;
    }

    void namco54xx::write(uint8_t data)
    {
	set_latched_timer->start(time_zero(), false, data);
	mcu_proc->fire_irq(true);
	irq_timer->start(irq_duration, false);

	// TODO: This really should be happening via the chip-select function
	// mcu_proc->fire_irq(line);
    }

    void namco54xx::set_reset_line(bool is_asserted)
    {
	is_reset = is_asserted;
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