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

#include "centipede.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(centiped)
	berrn_rom_region("maincpu", 0x10000)
	    berrn_rom_load("136001-407.d1", 0x2000, 0x0800)
	    berrn_rom_load("136001-408.e1", 0x2800, 0x0800)
	    berrn_rom_load("136001-409.fh1", 0x3000, 0x0800)
	    berrn_rom_load("136001-410.j1", 0x3800, 0x0800)
    berrn_rom_end

    CentipedeBase::CentipedeBase(berrndriver &drv) : driver(drv)
    {
	main_proc = new Berrn6502Processor(1512000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    driver.setScreen(bitmap);
	});

	interrupt_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    current_scanline += 1;

	    if (current_scanline & 16)
	    {
		if ((current_scanline - 1) & 32)
		{
		    cout << "Firing IRQ..." << endl;
		}
	    }

	    if (current_scanline == 256)
	    {
		current_scanline = 0;
	    }
	});

	bitmap = new BerrnBitmapRGB(256, 240);
	bitmap->clear();
    }

    CentipedeBase::~CentipedeBase()
    {

    }

    bool CentipedeBase::init_core()
    {
	scheduler.reset();
	main_proc->init();
	vblank_timer->start(time_in_hz(60), true);
	interrupt_timer->start((time_in_hz(60) / 256), true);
	scheduler.add_device(main_cpu);
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
	temp_vram.fill(0);
	driver.resize(640, 480, 1);
	return true;
    }

    void CentipedeBase::shutdown_core()
    {
	vblank_timer->stop();
	interrupt_timer->stop();
	main_proc->shutdown();
	scheduler.shutdown();
    }

    void CentipedeBase::run_core()
    {
	int64_t schedule_time = scheduler.get_current_time();
	int64_t frame_time = time_in_hz(60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}
    }

    void CentipedeBase::key_changed(BerrnInput key, bool is_pressed)
    {
	(void)key;
	(void)is_pressed;
	return;
    }

    uint8_t CentipedeBase::readCPU8(uint16_t addr)
    {
	addr &= 0x3FFF;
	uint8_t data = 0;

	if (inRange(addr, 0, 0x400))
	{
	    data = main_ram.at(addr);
	}
	else if (inRange(addr, 0x400, 0x800))
	{
	    data = temp_vram.at((addr & 0x3FF));
	}
	else if (addr == 0xC00)
	{
	    // TODO: IN0 reads
	    data = 0xBF;
	}
	else if (inRange(addr, 0x2000, 0x4000))
	{
	    data = main_rom.at(addr);
	}
	else if (isValidAddr(addr))
	{
	    data = readMem(addr);
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void CentipedeBase::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FFF;
	if (inRange(addr, 0, 0x400))
	{
	    main_ram.at(addr) = data;
	}
	else if (inRange(addr, 0x400, 0x800))
	{
	    temp_vram.at((addr & 0x3FF)) = data;
	}
	else if (inRange(addr, 0x1C00, 0x1C08))
	{
	    writeLatch((addr & 7), data);
	}
	else if (inRange(addr, 0x2000, 0x4000))
	{
	    return;
	}
	else if (isValidAddr(addr))
	{
	    writeMem(addr, data);
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    bool CentipedeBase::isValidAddr(uint16_t addr)
    {
	(void)addr;
	return false;
    }

    uint8_t CentipedeBase::readMem(uint16_t addr)
    {
	(void)addr;
	return 0;
    }

    void CentipedeBase::writeMem(uint16_t addr, uint8_t data)
    {
	(void)addr;
	(void)data;
	return;
    }

    void CentipedeBase::writeLatch(int addr, uint8_t data)
    {
	addr &= 7;
	bool line = testbit(data, 7);

	switch (addr)
	{
	    case 0: break;
	    case 1: break;
	    case 2: break;
	    case 3: break;
	    case 4: break;
	    default: writeLatchUpper(addr, line); break;
	}
    }

    void CentipedeBase::writeLatchUpper(int addr, bool line)
    {
	(void)addr;
	(void)line;
	return;
    }

    CentipedeCore::CentipedeCore(berrndriver &drv) : CentipedeBase(drv)
    {

    }

    CentipedeCore::~CentipedeCore()
    {

    }

    bool CentipedeCore::isValidAddr(uint16_t addr)
    {
	addr &= 0x3FFF;
	return inRange(addr, 0x1000, 0x1010);
    }

    uint8_t CentipedeCore::readMem(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x3FFF;

	if (inRange(addr, 0x1000, 0x1010))
	{
	    cout << "Reading value from Pokey address of " << hex << int(addr & 0xF) << endl;
	    exit(0);
	    data = 0;
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void CentipedeCore::writeMem(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FFF;
	if (inRange(addr, 0x1000, 0x1010))
	{
	    cout << "Writing value of " << hex << int(data) << " to Pokey address of " << hex << int(addr & 0xF) << endl;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    void CentipedeCore::writeLatchUpper(int addr, bool line)
    {
	switch (addr)
	{
	    case 7:
	    {
		if (line)
		{
		    cout << "Screen flipped" << endl;
		}
		else
		{
		    cout << "Screen is normal" << endl;
		}
	    }
	    break;
	    default: break;
	}
    }

    drivercentiped::drivercentiped()
    {
	core = new CentipedeCore(*this);
    }

    drivercentiped::~drivercentiped()
    {

    }

    string drivercentiped::drivername()
    {
	return "centiped";
    }

    bool drivercentiped::hasdriverROMs()
    {
	return true;
    }

    bool drivercentiped::drvinit()
    {
	if (!loadROM(berrn_rom_name(centiped)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivercentiped::drvshutdown()
    {
	core->shutdown_core();
    }
  
    void drivercentiped::drvrun()
    {
	core->run_core();
    }

    void drivercentiped::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }
};