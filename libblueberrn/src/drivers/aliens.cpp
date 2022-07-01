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

    AliensMainInterface::AliensMainInterface(berrndriver &drv, AliensCore &core) : driver(drv), main_core(core)
    {

    }

    AliensMainInterface::~AliensMainInterface()
    {

    }

    void AliensMainInterface::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
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
	else if (inRange(addr, 0x0000, 0x0400))
	{
	    data = main_core.readBank0000(addr);
	}
	else if (inRange(addr, 0x0400, 0x2000))
	{
	    data = main_ram.at(addr - 0x400);
	}
	else if (addr == 0x5F88)
	{
	    // Watchdog timer (unimplemented)
	    data = 0x00;
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void AliensMainInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0x8000, 0x10000))
	{
	    return;
	}
	else if (inRange(addr, 0x0000, 0x0400))
	{
	    main_core.writeBank0000(addr, data);
	}
	else if (inRange(addr, 0x0400, 0x2000))
	{
	    main_ram.at(addr - 0x400) = data;
	}
	else if (addr == 0x5F88)
	{
	    main_core.writeCoinCounter(data);
	}
	else if (addr == 0x5F8C)
	{
	    cout << "Writing value of " << hex << int(data) << " to sound IRQ trigger register" << endl;
	}
	else if (inRange(addr, 0x4000, 0x8000))
	{
	    cout << "Writing value of " << hex << int(data) << " to K052109/K051960 address of " << hex << int(addr - 0x4000) << endl;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    AliensCore::AliensCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new AliensMainInterface(driver, *this);
	main_cpu = new BerrnKonami2CPU(driver, 3000000, *main_inter);
    }

    AliensCore::~AliensCore()
    {

    }

    bool AliensCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_cpu->init();
	scheduler.add_device(main_cpu);
	palette_ram.fill(0);
	bank_0000_ram.fill(0);
	is_bank_0000_ram = false;
	return true;
    }

    void AliensCore::stop_core()
    {
	main_cpu->shutdown();
	main_inter->shutdown();
    }

    void AliensCore::run_core()
    {
	driver.run_scheduler();
    }

    uint8_t AliensCore::readBank0000(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x3FF;

	if (is_bank_0000_ram)
	{
	    data = bank_0000_ram.at(addr);
	}
	else
	{
	    data = palette_ram.at(addr);
	}

	return data;
    }

    void AliensCore::writeBank0000(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FF;

	if (is_bank_0000_ram)
	{
	    bank_0000_ram.at(addr) = data;
	}
	else
	{
	    palette_ram.at(addr) = data;
	}
    }

    void AliensCore::writeCoinCounter(uint8_t data)
    {
	string rmrd_line = testbit(data, 6) ? "Asserting" : "Clearing";
	is_bank_0000_ram = testbit(data, 5);
	cout << rmrd_line << " K052109 RMRD line..." << endl;
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