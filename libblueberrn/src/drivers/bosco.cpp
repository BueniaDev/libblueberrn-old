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

#include "bosco.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(bosco)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("bos3_1.3n", 0x0000, 0x1000)
	    berrn_rom_load("bos1_2.3m", 0x1000, 0x1000)
	    berrn_rom_load("bos1_3.3l", 0x2000, 0x1000)
	    berrn_rom_load("bos1_4b.3k", 0x3000, 0x1000)
    berrn_rom_end

    BoscoInterface::BoscoInterface(berrndriver &drv, BoscoCore &core) : driver(drv), shared_core(core)
    {
	
    }

    BoscoInterface::~BoscoInterface()
    {

    }

    void BoscoInterface::init(string tag)
    {
	main_rom = driver.get_rom_region(tag);
    }

    void BoscoInterface::shutdown()
    {
	main_rom.clear();
    }

    uint8_t BoscoInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x4000)
	{
	    data = main_rom.at(addr);
	}
	else
	{
	    data = shared_core.readByte(addr);
	}

	return data;
    }

    void BoscoInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x4000)
	{
	    return;
	}
	else
	{
	    shared_core.writeByte(addr, data);
	}
    }

    BoscoCore::BoscoCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new BoscoInterface(driver, *this);
	main_proc = new BerrnZ80Processor(3072000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	n06xx_0 = new namco06xx(driver, 48000);
    }

    BoscoCore::~BoscoCore()
    {

    }

    uint8_t BoscoCore::readByte(uint16_t addr)
    {
	uint8_t data = 0;

	if (false)
	{
	}
	else
	{
	    cout << "Reading from Bosco address of " << hex << int(addr) << endl;
	    exit(0);
	}

	return data;
    }

    void BoscoCore::writeByte(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0x6820, 0x6828))
	{
	    writeIO(addr, testbit(data, 0));
	}
	else if (addr == 0x7100)
	{
	    n06xx_0->writeControl(data);
	}
	else if (inRange(addr, 0x8000, 0x9000))
	{
	    video_ram.at((addr & 0xFFF)) = data;
	}
	else if (inRange(addr, 0x9000, 0x9100))
	{
	    return;
	}
	else if (addr == 0x9100)
	{
	    return;
	}
	else if (addr == 0x9830)
	{
	    // TODO: Implement starfield control register
	    return;
	}
	else
	{
	    cout << "Writing value of " << hex << int(data) << " to Bosco address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    void BoscoCore::writeIO(int addr, bool line)
    {
	addr &= 7;

	switch (addr)
	{
	    case 0:
	    {
		if (line)
		{
		    cout << "Asserting line 0..." << endl;
		}
		else
		{
		    cout << "Clearing line 0..." << endl;
		}
	    }
	    break;
	    case 1:
	    {
		if (line)
		{
		    cout << "Asserting line 1..." << endl;
		}
		else
		{
		    cout << "Clearing line 1..." << endl;
		}
	    }
	    break;
	    case 2:
	    {
		if (line)
		{
		    cout << "Asserting line 2..." << endl;
		}
		else
		{
		    cout << "Clearing line 2..." << endl;
		}
	    }
	    break;
	    case 3:
	    {
		if (line)
		{
		    cout << "Asserting line 3..." << endl;
		}
		else
		{
		    cout << "Clearing line 3..." << endl;
		}
	    }
	    break;
	}
    }

    bool BoscoCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	scheduler.set_quantum(time_in_hz(6000));
	main_inter->init("maincpu");
	main_proc->init();
	scheduler.add_device(main_cpu);
	n06xx_0->set_maincpu(main_cpu);
	video_ram.fill(0);
	return true;
    }

    void BoscoCore::stop_core()
    {
	main_proc->shutdown();
	main_inter->shutdown();
    }

    void BoscoCore::run_core()
    {
	driver.run_scheduler();
    }

    driverbosco::driverbosco()
    {
	core = new BoscoCore(*this);
    }

    driverbosco::~driverbosco()
    {

    }

    string driverbosco::drivername()
    {
	return "bosco";
    }

    bool driverbosco::drvinit()
    {
	if (!loadROM(berrn_rom_name(bosco)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverbosco::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverbosco::drvrun()
    {
	core->run_core();
    }

    void driverbosco::keychanged(BerrnInput key, bool is_pressed)
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