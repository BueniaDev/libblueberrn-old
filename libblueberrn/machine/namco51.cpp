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

#include "namco51.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(namco51)
      berrn_rom_region("mcu51", 0x400)
        berrn_rom_load("51xx.bin", 0x0000, 0x0400)
    berrn_rom_end

    namco51xxInterface::namco51xxInterface(berrndriver &drv) : driver(drv)
    {

    }

    namco51xxInterface::~namco51xxInterface()
    {

    }

    void namco51xxInterface::init()
    {
	mcu_rom = driver.get_rom_region("mcu51");
	mcu_data.fill(0);
	portO = 0;
	m_rw = false;
    }

    void namco51xxInterface::shutdown()
    {
	mcu_rom.clear();
    }

    uint8_t namco51xxInterface::readOp8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x400)
	{
	    data = mcu_rom.at(addr);
	}

	return data;
    }

    uint8_t namco51xxInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x40)
	{
	    data = mcu_data.at(addr);
	}

	return data;
    }

    void namco51xxInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x40)
	{
	    mcu_data.at(addr) = data;
	}
    }

    uint8_t namco51xxInterface::portIn(uint16_t port)
    {
	uint8_t data = 0x00;

	switch (port)
	{
	    case 0:
	    case 1:
	    case 2:
	    case 3:
	    {
		if (input_func)
		{
		    data = input_func(port);
		}
		else
		{
		    data = BerrnInterface::portIn(port);
		}
	    }
	    break;
	    case 4: data = ((m_rw << 3) | (portO & 0x7)); break;
	    default: data = BerrnInterface::portIn(port); break;
	}

	return data;
    }

    void namco51xxInterface::portOut(uint16_t port, uint8_t data)
    {
	switch (port)
	{
	    case 4:
	    {
		uint8_t out = (data & 0xF);

		if (testbit(data, 4))
		{
		    portO = ((portO & 0xF) | (out << 4));
		}
		else
		{
		    portO = ((portO & 0xF0) | out);
		}
	    }
	    break;
	    case 5:
	    {
		if (output_func)
		{
		    output_func(0, data);
		}
	    }
	    break;
	    default: BerrnInterface::portOut(port, data); break;
	}
    }

    void namco51xxInterface::rw(bool line)
    {
	m_rw = line;
    }

    uint8_t namco51xxInterface::read()
    {
	return portO;
    }

    void namco51xxInterface::write(uint8_t data)
    {
	portO = data;
    }

    namco51xx::namco51xx(berrndriver &drv, BerrnScheduler &sched, uint64_t clk_freq) : driver(drv), scheduler(sched)
    {
	mcu_inter = new namco51xxInterface(driver);
	mcu_proc = new BerrnMB8843Processor(clk_freq, *mcu_inter);
	mcu_cpu = new BerrnCPU(scheduler, *mcu_proc);

	write_sync = new BerrnTimer("WSync", scheduler, [&](int64_t param, int64_t)
	{
	    uint8_t data = uint8_t(param);
	    mcu_inter->write(data);
	});
    }

    namco51xx::~namco51xx()
    {

    }

    bool namco51xx::init()
    {
	if (!driver.loadROM(berrn_rom_name(namco51)))
	{
	    return false;
	}

	mcu_inter->init();
	mcu_proc->init();
	stack.fill(0);
	sp = 0;
	scheduler.add_device(mcu_cpu);
	return true;
    }

    void namco51xx::shutdown()
    {
	mcu_inter->shutdown();
	scheduler.remove_device(mcu_cpu);
    }

    void namco51xx::set_input_callback(berrncbread8 cb)
    {
	mcu_inter->set_input_callback(cb);
    }

    void namco51xx::set_output_callback(berrncbwrite8 cb)
    {
	mcu_inter->set_output_callback(cb);
    }

    void namco51xx::vblank()
    {
	if (is_reset)
	{
	    return;
	}

	// Pulse the MCU clock line
	mcu_proc->clock_write(false);
	mcu_proc->clock_write(true);
    }

    void namco51xx::chip_select(bool line)
    {
	mcu_proc->fire_irq(line);
    }

    void namco51xx::rw(bool line)
    {
	mcu_inter->rw(line);
    }

    uint8_t namco51xx::read()
    {
	uint8_t data = mcu_inter->read();
	return data;
    }

    void namco51xx::write(uint8_t data)
    {
	write_sync->start(time_zero(), false, data);
    }

    void namco51xx::set_reset_line(bool is_asserted)
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