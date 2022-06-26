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

#include "gradius.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(gradius)
	berrn_rom_region("maincpu", 0xC0000, 0)
	    berrn_rom_load16_byte("400-a06.15l", 0x00000, 0x08000)
	    berrn_rom_load16_byte("400-a04.10l", 0x00001, 0x08000)
	    berrn_rom_load16_byte("456-a07.17l", 0x80000, 0x20000)
	    berrn_rom_load16_byte("456-a05.12l", 0x80001, 0x20000)
    berrn_rom_end

    GradiusM68K::GradiusM68K(berrndriver &drv, GradiusCore &core) : driver(drv), main_core(core)
    {

    }

    GradiusM68K::~GradiusM68K()
    {

    }

    void GradiusM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
    }

    void GradiusM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t GradiusM68K::readCPU16(bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;
	if (addr < 0x10000)
	{
	    if (upper)
	    {
		data |= (main_rom.at(addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_rom.at(addr + 1);
	    }
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    GradiusCore::GradiusCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new GradiusM68K(driver, *this);
	main_cpu = new BerrnM68KCPU(driver, 9216000, *main_inter);
    }

    GradiusCore::~GradiusCore()
    {

    }

    bool GradiusCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_cpu->init();
	scheduler.add_device(main_cpu);
	return true;
    }

    void GradiusCore::stop_core()
    {
	main_cpu->shutdown();
	main_inter->shutdown();
    }

    void GradiusCore::run_core()
    {
	driver.run_scheduler();
    }

    drivergradius::drivergradius()
    {
	core = new GradiusCore(*this);
    }

    drivergradius::~drivergradius()
    {

    }

    string drivergradius::drivername()
    {
	return "gradius";
    }

    string drivergradius::parentname()
    {
	return "nemesis";
    }

    bool drivergradius::drvinit()
    {
	if (!loadROM(berrn_rom_name(gradius)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivergradius::drvshutdown()
    {
	core->stop_core();
    }
  
    void drivergradius::drvrun()
    {
	core->run_core();
    }

    void drivergradius::keychanged(BerrnInput key, bool is_pressed)
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