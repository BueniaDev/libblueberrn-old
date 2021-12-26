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

#include "rastan.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(rastan)
	berrn_rom_region("maincpu", 0x60000)
	    berrn_rom_load16_byte("b04-38.19",  0x00000, 0x10000)
	    berrn_rom_load16_byte("b04-37.7",   0x00001, 0x10000)
	    berrn_rom_load16_byte("b04-40.20",  0x20000, 0x10000)
	    berrn_rom_load16_byte("b04-39.8",   0x20001, 0x10000)
	    berrn_rom_load16_byte("b04-42.21",  0x40000, 0x10000)
	    berrn_rom_load16_byte("b04-43-1.9", 0x40001, 0x10000)
    berrn_rom_end

    RastanCore::RastanCore(berrndriver &drv) : driver(drv)
    {
	scheduler.set_interleave(10);
	main_proc = new BerrnM68KProcessor(8000000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	bitmap = new BerrnBitmapRGB(320, 240);
	bitmap->clear();
    }

    RastanCore::~RastanCore()
    {

    }

    bool RastanCore::init_core()
    {
	m68k_rom = driver.get_rom_region("maincpu");
	scheduler.reset();
	scheduler.add_device(main_cpu);
	main_proc->init();

	driver.resize(320, 240, 2);

	return true;
    }

    void RastanCore::shutdown_core()
    {
	main_proc->shutdown();
	scheduler.remove_device(main_cpu);
    }

    void RastanCore::run_core()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = time_in_hz(60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}

	// Placeholder code
	driver.setScreen(bitmap);
    }

    uint16_t RastanCore::readCPU16(bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;
	if (addr < 0x60000)
	{
	    if (upper)
	    {
		data |= (m68k_rom.at(addr) << 8);
	    }

	    if (lower)
	    {
		data |= m68k_rom.at(addr + 1);
	    }
	}
	else if (inRange(addr, 0x10C000, 0x110000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);

	    if (upper)
	    {
		data |= (m68k_ram.at(ram_addr) << 8);
	    }

	    if (lower)
	    {
		data |= m68k_ram.at(ram_addr + 1);
	    }
	}
	else if (inRange(addr, 0x200000, 0x201000))
	{
	    /*
	    cout << "Reading palette RAM..." << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    */
	    data = 0x0000;
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    void RastanCore::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x60000)
	{
	    return;
	}
	else if (inRange(addr, 0x10C000, 0x110000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);

	    if (upper)
	    {
		m68k_ram.at(ram_addr) = (data >> 8);
	    }

	    if (lower)
	    {
		m68k_ram.at(ram_addr + 1) = (data & 0xFF);
	    }
	}
	else if (inRange(addr, 0x200000, 0x201000))
	{
	    /*
	    cout << "Writing to palette RAM..." << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (addr == 0x350008)
	{
	    return;
	}
	else if (addr == 0x380000)
	{
	    /*
	    cout << "Sprite control write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	}
	else if (addr == 0x3C0000)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (addr == 0x3E0000)
	{
	    if (!lower)
	    {
		// Even byte writes are ignored
		return;
	    }

	    // cout << "Writing value of " << hex << int((data & 0xFF)) << " to Taito PC060HA master port" << endl;
	}
	else if (addr == 0x3E0002)
	{
	    if (!lower)
	    {
		// Even byte writes are ignored
		return;
	    }

	    // cout << "Writing value of " << hex << int((data & 0xFF)) << " to Taito PC060HA master communication port" << endl;
	}
	else if (inRange(addr, 0xC50000, 0xC50004))
	{
	    /*
	    cout << "Taito PC080SN write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	}
	else if (inRange(addr, 0xD00000, 0xD04000))
	{
	    /*
	    cout << "Taito PC090OJ write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	}
	else
	{
	    BerrnInterface::writeCPU16(upper, lower, addr, data);
	}
    }

    driverrastan::driverrastan()
    {
	core = new RastanCore(*this);
    }

    driverrastan::~driverrastan()
    {

    }

    string driverrastan::drivername()
    {
	return "rastan";
    }

    bool driverrastan::hasdriverROMs()
    {
	return true;
    }

    bool driverrastan::drvinit()
    {
	if (!loadROM(berrn_rom_name(rastan)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverrastan::drvshutdown()
    {
	core->shutdown_core();
    }

    void driverrastan::drvrun()
    {
	core->run_core();
    }

    void driverrastan::keychanged(BerrnInput key, bool is_pressed)
    {
	(void)key;
	(void)is_pressed;
	return;
    }
};

