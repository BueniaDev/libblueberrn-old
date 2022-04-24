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

#include "xmen.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(xmen)
	berrn_rom_region("maincpu", 0x100000, 0)
	    berrn_rom_load16_byte("065-eba04.10d", 0x00000, 0x20000)
	    berrn_rom_load16_byte("065-eba05.10f", 0x00001, 0x20000)
	    berrn_rom_load16_byte("065-a02.9d",    0x80000, 0x40000)
	    berrn_rom_load16_byte("065-a03.9f",    0x80001, 0x40000)
    berrn_rom_end

    XMenM68K::XMenM68K(berrndriver &drv, XMenCore &core) : driver(drv), main_core(core)
    {

    }

    XMenM68K::~XMenM68K()
    {

    }

    void XMenM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void XMenM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t XMenM68K::readCPU16(bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;
	if (addr < 0x40000)
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
	else if (inRange(addr, 0x80000, 0x100000))
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
	else if (addr == 0x10A004)
	{
	    if (upper)
	    {
		data |= 0xFF00;
	    }

	    if (lower)
	    {
		data |= 0x3F;
	    }
	}
	else if (inRange(addr, 0x110000, 0x114000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);
	    if (upper)
	    {
		data |= (main_ram.at(ram_addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_ram.at(ram_addr + 1);
	    }
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    void XMenM68K::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x40000)
	{
	    return;
	}
	else if (inRange(addr, 0x80000, 0x100000))
	{
	    return;
	}
	else if (addr == 0x108000)
	{
	    if (upper)
	    {
		if (testbit(data, 9))
		{
		    cout << "Asserting K052109 RM/RD line..." << endl;
		}
		else
		{
		    cout << "Clearing K052109 RM/RD line..." << endl;
		}
	    }

	    if (lower)
	    {
		cout << "Writing value of " << hex << int(data & 0xFF) << " to EEPROM" << endl;
	    }
	}
	else if (inRange(addr, 0x108020, 0x108028))
	{
	    cout << "K053246 write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	}
	else if (inRange(addr, 0x108040, 0x108060))
	{
	    if (lower)
	    {
		cout << "Writing value of " << hex << int(data & 0xFF) << " to K054321 register of " << hex << int(addr) << endl;
	    }
	}
	else if (inRange(addr, 0x108060, 0x108080))
	{
	    if (lower)
	    {
		cout << "Writing value of " << hex << int(data & 0xFF) << " to K053251 register of " << hex << int(addr) << endl;
	    }
	}
	else if (addr == 0x10A000)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (inRange(addr, 0x110000, 0x114000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);
	    if (upper)
	    {
		main_ram.at(ram_addr) = (data >> 8);
	    }

	    if (lower)
	    {
		main_ram.at(ram_addr + 1) = (data & 0xFF);
	    }
	}
	else if (addr == 0x18FA00)
	{
	    if (lower)
	    {
		main_core.writeIRQ(testbit(data, 2));
	    }
	}
	else if (inRange(addr, 0x18C000, 0x198000))
	{
	    cout << "K052109 write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	}
	else
	{
	    BerrnInterface::writeCPU16(upper, lower, addr, data);
	}
    }

    XMenCore::XMenCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new XMenM68K(driver, *this);
	main_proc = new BerrnM68KProcessor(16000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    return;
	});

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    if (current_scanline == 0)
	    {
		cout << "Sprite DMA IRQ..." << endl;
		main_proc->fire_interrupt_level(5);
	    }

	    if ((current_scanline == 240) && is_vblank_irq)
	    {
		cout << "Possible VBlank IRQ..." << endl;
		exit(0);
	    }
	});
    }

    XMenCore::~XMenCore()
    {

    }

    bool XMenCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_proc->init();

	vblank_timer->start(time_in_hz(59.17), true);
	irq_timer->start((time_in_hz(59.17) / 256), true);

	scheduler.add_device(main_cpu);

	return true;
    }

    void XMenCore::stop_core()
    {
	main_proc->shutdown();
	main_inter->shutdown();
	vblank_timer->stop();
	irq_timer->stop();
    }

    void XMenCore::run_core()
    {
	driver.run_scheduler();
    }

    void XMenCore::writeIRQ(bool line)
    {
	is_vblank_irq = line;
    }

    driverxmen::driverxmen()
    {
	core = new XMenCore(*this);
    }

    driverxmen::~driverxmen()
    {

    }

    string driverxmen::drivername()
    {
	return "xmen";
    }

    bool driverxmen::drvinit()
    {
	if (!loadROM(berrn_rom_name(xmen)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverxmen::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverxmen::drvrun()
    {
	core->run_core();
    }

    void driverxmen::keychanged(BerrnInput key, bool is_pressed)
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