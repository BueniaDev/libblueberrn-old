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

#include "1943.h"
using namespace berrn;

// Driver Name:
// 1943: The Battle of Midway - Capcom (WIP)
//
// Blueberry's Notes:
//
// This driver is a huge WIP, and a lot of features (including the video system)
// are completely unimplemented.
// However, work is being done to improve this driver, so don't lose hope here!

namespace berrn
{
    berrn_rom_start(1943u)
	berrn_rom_region("maincpu", 0x30000, 0)
	    berrn_rom_load("bmu01c.12d", 0x00000, 0x08000)
	    berrn_rom_load("bmu02c.13d", 0x10000, 0x10000)
	    berrn_rom_load("bmu03c.14d", 0x20000, 0x10000)
    berrn_rom_end

    Berrn1943Main::Berrn1943Main(berrndriver &drv, Berrn1943Core &core) : driver(drv), main_core(core)
    {

    }

    Berrn1943Main::~Berrn1943Main()
    {

    }

    void Berrn1943Main::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);

	// TODO: Implement video system
	obj_ram.fill(0);
	color_ram.fill(0);
	video_ram.fill(0);
    }

    void Berrn1943Main::shutdown()
    {
	main_rom.clear();
    }

    uint8_t Berrn1943Main::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x8000)
	{
	    data = main_rom.at(addr);
	}
	else if (addr == 0xC000)
	{
	    // SYSTEM
	    data = 0xFF;
	}
	else if (addr == 0xC003)
	{
	    // DSWA (Note: return 0x78 here to activate test mode)
	    data = 0xF8;
	}
	else if (addr == 0xC004)
	{
	    // DSWB
	    data = 0xFF;
	}
	else if (inRange(addr, 0xD000, 0xD400))
	{
	    data = video_ram.at(addr & 0x3FF);
	}
	else if (inRange(addr, 0xD400, 0xD800))
	{
	    data = color_ram.at(addr & 0x3FF);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    data = main_ram.at(addr & 0xFFF);
	}
	else if (inRangeEx(addr, 0xF000, 0xFFFF))
	{
	    data = obj_ram.at(addr & 0xFFF);
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void Berrn1943Main::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0xC000)
	{
	    return;
	}
	else if (addr == 0xC800)
	{
	    // Sound latch write (currently unimplemented)
	    return;
	}
	else if (addr == 0xC804)
	{
	    current_rom_bank = ((data >> 2) & 0x7);
	}
	else if (addr == 0xC806)
	{
	    // Watchdog timer (not implemented)
	    return;
	}
	else if (addr == 0xD800)
	{
	    scrollx = ((scrollx & 0xFF00) | data);
	}
	else if (addr == 0xD801)
	{
	    scrollx = ((scrollx & 0xFF) | (data << 8));
	}
	else if (addr == 0xD802)
	{
	    scrolly = data;
	}
	else if (addr == 0xD803)
	{
	    bg_scrollx = ((bg_scrollx & 0xFF00) | data);
	}
	else if (addr == 0xD804)
	{
	    bg_scrollx = ((bg_scrollx & 0xFF) | (data << 8));
	}
	else if (addr == 0xD806)
	{
	    return;
	}
	else if (inRange(addr, 0xD000, 0xD400))
	{
	    video_ram.at(addr & 0x3FF) = data;
	}
	else if (inRange(addr, 0xD400, 0xD800))
	{
	    color_ram.at(addr & 0x3FF) = data;
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    main_ram.at(addr & 0xFFF) = data;
	}
	else if (inRangeEx(addr, 0xF000, 0xFFFF))
	{
	    obj_ram.at(addr & 0xFFF) = data;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    Berrn1943Core::Berrn1943Core(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new Berrn1943Main(driver, *this);
	main_proc = new BerrnZ80Processor(6000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    main_proc->fire_interrupt8(0xFF);
	});
    }

    Berrn1943Core::~Berrn1943Core()
    {

    }

    bool Berrn1943Core::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_proc->init();
	scheduler.add_device(main_cpu);
	irq_timer->start(time_in_hz(60), true);
	return true;
    }

    void Berrn1943Core::stop_core()
    {
	irq_timer->stop();
	main_proc->shutdown();
	main_inter->shutdown();
    }

    void Berrn1943Core::run_core()
    {
	driver.run_scheduler();
    }

    driver1943u::driver1943u()
    {
	core = new Berrn1943Core(*this);
    }

    driver1943u::~driver1943u()
    {

    }

    string driver1943u::drivername()
    {
	return "1943u";
    }

    string driver1943u::parentname()
    {
	return "1943";
    }

    bool driver1943u::drvinit()
    {
	if (!loadROM(berrn_rom_name(1943u)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driver1943u::drvshutdown()
    {
	core->stop_core();
    }
  
    void driver1943u::drvrun()
    {
	core->run_core();
    }

    void driver1943u::keychanged(BerrnInput key, bool is_pressed)
    {
	string key_state = (is_pressed) ? "pressed" : "released";

	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		cout << "Coin button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		cout << "P1 start button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		cout << "P1 left button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		cout << "P1 right button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnUpP1:
	    {
		cout << "P1 up button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnDownP1:
	    {
		cout << "P1 down button has been " << key_state << endl;
	    }
	    break;
	    case BerrnInput::BerrnFireP1:
	    {
		cout << "P1 fire button has been " << key_state << endl;
	    }
	    break;
	    default: break;
	}
    }
};