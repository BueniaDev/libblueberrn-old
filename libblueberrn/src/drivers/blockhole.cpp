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

#include "blockhole.h"
using namespace berrn;

// Block Hole / Quarth (WIP)

namespace berrn
{
    berrn_rom_start(blockhl)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("973l02.e21", 0x00000, 0x10000)
    berrn_rom_end

    BlockHoleMainInterface::BlockHoleMainInterface(berrndriver &drv, BlockHoleCore &core) : driver(drv), main_core(core)
    {

    }

    BlockHoleMainInterface::~BlockHoleMainInterface()
    {

    }

    void BlockHoleMainInterface::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void BlockHoleMainInterface::shutdown()
    {
	main_rom.clear();
    }

    uint8_t BlockHoleMainInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (inRange(addr, 0x8000, 0x10000))
	{
	    uint32_t rom_addr = ((addr - 0x8000) + 0x8000);
	    data = main_rom.at(rom_addr);
	}
	else if (inRangeEx(addr, 0x1F94, 0x1F98))
	{
	    data = main_core.readDIP(addr - 0x1F94);
	}
	else if (inRange(addr, 0x0000, 0x4000))
	{
	    cout << "Reading value from K052109/K051960 address of " << hex << int(addr) << endl;
	    data = 0;
	}
	else if (inRange(addr, 0x4000, 0x5800))
	{
	    data = main_ram.at(addr - 0x4000);
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void BlockHoleMainInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0x6000, 0x10000))
	{
	    return;
	}
	else if (addr == 0x1F84)
	{
	    cout << "Writing value of " << hex << int(data) << " to sound latch" << endl;
	}
	else if (addr == 0x1F88)
	{
	    cout << "Writing value of " << hex << int(data) << " to sound IRQ register" << endl;
	}
	else if (addr == 0x1F8C)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (inRange(addr, 0x0000, 0x4000))
	{
	    cout << "Writing value of " << hex << int(data) << " to K052109/K051960 address of " << hex << int(addr) << endl;
	}
	else if (inRange(addr, 0x4000, 0x5800))
	{
	    main_ram.at(addr - 0x4000) = data;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    void BlockHoleMainInterface::portOut(uint16_t port, uint8_t data)
    {
	(void)port;
	current_rom_bank = (data & 0x3);
	main_core.setLines(data);
    }

    BlockHoleCore::BlockHoleCore(berrndriver &drv) : driver(drv)
    {
	main_inter = new BlockHoleMainInterface(driver, *this);
	main_cpu = new BerrnKonami2CPU(driver, 3000000, *main_inter);
    }

    BlockHoleCore::~BlockHoleCore()
    {

    }

    bool BlockHoleCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_cpu->init();

	scheduler.add_device(main_cpu);
	return true;
    }

    void BlockHoleCore::stop_core()
    {
	main_inter->shutdown();
	main_cpu->shutdown();
    }

    void BlockHoleCore::run_core()
    {
	driver.run_scheduler();
    }

    uint8_t BlockHoleCore::readDIP(int bank)
    {
	uint8_t data = 0;
	switch (bank)
	{
	    case 0: data = 0xFF; break; // DSW3
	    default:
	    {
		cout << "Reading DIP switch of " << hex << int(bank) << endl;
		exit(0);
	    }
	    break;
	}

	return data;
    }

    void BlockHoleCore::setLines(uint8_t data)
    {
	string bank_str = testbit(data, 5) ? "Asserting" : "Clearing";
	string rmrd_str = testbit(data, 6) ? "Asserting" : "Clearing";
	cout << bank_str << " bank 5800 line..." << endl;
	cout << rmrd_str << " K052109 RM/RD line..." << endl;
    }

    driverblockhl::driverblockhl()
    {
	core = new BlockHoleCore(*this);
    }

    driverblockhl::~driverblockhl()
    {

    }

    string driverblockhl::drivername()
    {
	return "blockhl";
    }

    bool driverblockhl::drvinit()
    {
	if (!loadROM(berrn_rom_name(blockhl)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverblockhl::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverblockhl::drvrun()
    {
	core->run_core();
    }

    void driverblockhl::keychanged(BerrnInput key, bool is_pressed)
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