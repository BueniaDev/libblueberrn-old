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

#include "aliens.h"
using namespace berrn;

// Aliens (WIP)

namespace berrn
{
    berrn_rom_start(aliens)
	berrn_rom_region("maincpu", 0x30000, 0)
	    berrn_rom_load("875_j01.c24", 0x00000, 0x20000)
	    berrn_rom_load("875_j02.e24", 0x20000, 0x10000)
    berrn_rom_end

    AliensMainInterface::AliensMainInterface(berrndriver &drv, AliensCore &cb) : driver(drv), core(cb)
    {

    }

    AliensMainInterface::~AliensMainInterface()
    {

    }

    void AliensMainInterface::init()
    {
	main_rom = driver.get_rom_region("maincpu");
    }

    void AliensMainInterface::shutdown()
    {
	main_rom.clear();
    }

    uint8_t AliensMainInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (inRange(addr, 0x8000, 0x10000))
	{
	    uint32_t rom_addr = ((addr - 0x8000) + 0x28000);
	    data = main_rom.at(rom_addr); 
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    AliensCore::AliensCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new AliensMainInterface(driver, *this);
	main_proc = new BerrnKonami2Processor(3000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);
    }

    AliensCore::~AliensCore()
    {

    }

    bool AliensCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_proc->init();
	scheduler.add_device(main_cpu);
	return true;
    }

    void AliensCore::stop_core()
    {
	main_proc->shutdown();
	main_inter->shutdown();
    }

    void AliensCore::run_core()
    {
	driver.run_scheduler();
    }

    driveraliens::driveraliens()
    {
	core = new AliensCore(*this);
    }

    driveraliens::~driveraliens()
    {

    }

    string driveraliens::drivername()
    {
	return "aliens";
    }

    bool driveraliens::drvinit()
    {
	if (!loadROM(berrn_rom_name(aliens)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driveraliens::drvshutdown()
    {
	core->stop_core();
    }
  
    void driveraliens::drvrun()
    {
	core->run_core();
    }

    void driveraliens::keychanged(BerrnInput key, bool is_pressed)
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
	    case BerrnInput::BerrnButton1P1:
	    {
		cout << "P1 button 1 has been " << key_state << endl;
	    }
	    break;
	    default: break;
	}
    }
};