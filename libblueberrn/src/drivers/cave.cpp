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

#include "cave.h"
using namespace berrn;

// Driver Name:
// Cave 68K Hardware - Cave (WIP)
//
// Blueberry's Notes:
//
// This driver is a huge WIP, and a lot of features (including the video system)
// are completely unimplemented.
// However, work is being done to improve this driver, so don't lose hope here!

namespace berrn
{
    // TODO: Load in other ROMs in this set as well
    // (currently only the M68K ROMs are included)
    berrn_rom_start(ddonpach)
	berrn_rom_region("maincpu", 0x100000, 0)
	    berrn_rom_load16_byte("b1.u27", 0x000000, 0x080000)
	    berrn_rom_load16_byte("b2.u26", 0x000001, 0x080000)
    berrn_rom_end

    CaveM68K::CaveM68K(berrndriver &drv, CaveCore &core) : driver(drv), main_core(core)
    {

    }

    CaveM68K::~CaveM68K()
    {

    }

    DDonPachiM68K::DDonPachiM68K(berrndriver &drv, CaveCore &core) : CaveM68K(drv, core)
    {

    }

    DDonPachiM68K::~DDonPachiM68K()
    {

    }

    void DDonPachiM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
	sprite_ram.fill(0);
    }

    void DDonPachiM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t DDonPachiM68K::readCPU16(bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;

	if (addr < 0x100000)
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
	else if (inRange(addr, 0x100000, 0x110000))
	{
	    if (upper)
	    {
		data |= (main_ram.at(addr & 0xFFFF) << 8);
	    }

	    if (lower)
	    {
		data |= main_ram.at((addr & 0xFFFF) + 1);
	    }
	}
	else if (inRange(addr, 0x400000, 0x410000))
	{
	    if (upper)
	    {
		data |= (sprite_ram.at(addr & 0xFFFF) << 8);
	    }

	    if (lower)
	    {
		data |= sprite_ram.at((addr & 0xFFFF) + 1);
	    }
	}
	else if (inRange(addr, 0x500000, 0x508000))
	{
	    /*
	    cout << "Tilemap 0 VRAM read" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    data = 0;
	}
	else if (inRange(addr, 0x600000, 0x608000))
	{
	    /*
	    cout << "Tilemap 1 VRAM read" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    */
	    data = 0;
	}
	else if (inRange(addr, 0x700000, 0x710000))
	{
	    /*
	    cout << "Tilemap 2 VRAM read" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    */
	    data = 0;
	}
	else if (addr == 0xD00000)
	{
	    // TODO: IN0 reads
	    if (upper)
	    {
		data |= 0xFF00;
	    }

	    if (lower)
	    {
		data |= 0xFF;
	    }
	}
	else if (addr == 0xD00002)
	{
	    // TODO: IN1 reads
	    if (upper)
	    {
		data |= 0xF700;
	    }

	    if (lower)
	    {
		data |= 0xFF;
	    }
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    void DDonPachiM68K::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x100000)
	{
	    return;
	}
	else if (inRange(addr, 0x100000, 0x110000))
	{
	    if (upper)
	    {
		main_ram.at((addr & 0xFFFF)) = (data >> 8);
	    }

	    if (lower)
	    {
		main_ram.at((addr & 0xFFFF) + 1) = (data & 0xFF);
	    }
	}
	else if (inRange(addr, 0x400000, 0x410000))
	{
	    if (upper)
	    {
		sprite_ram.at((addr & 0xFFFF)) = (data >> 8);
	    }

	    if (lower)
	    {
		sprite_ram.at((addr & 0xFFFF) + 1) = (data & 0xFF);
	    }
	}
	else if (inRange(addr, 0x500000, 0x508000))
	{
	    /*
	    cout << "Tilemap 0 VRAM write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (inRange(addr, 0x600000, 0x608000))
	{
	    /*
	    cout << "Tilemap 1 VRAM write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (inRange(addr, 0x700000, 0x710000))
	{
	    /*
	    cout << "Tilemap 2 VRAM write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (inRange(addr, 0x800000, 0x800080))
	{
	    /*
	    cout << "Video register write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (inRange(addr, 0x900000, 0x900006))
	{
	    /*
	    cout << "Tilemap 038 layer 0 register write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (inRange(addr, 0xA00000, 0xA00006))
	{
	    /*
	    cout << "Tilemap 038 layer 1 register write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (inRange(addr, 0xB00000, 0xB00006))
	{
	    /*
	    cout << "Tilemap 038 layer 2 register write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (addr == 0xE00000)
	{
	    /*
	    cout << "EEPROM write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else
	{
	    BerrnInterface::writeCPU16(upper, lower, addr, data);
	}
    }

    CaveCore::CaveCore(berrndriver &drv, CaveM68K &inter) : driver(drv), interface(inter)
    {
	auto &scheduler = driver.get_scheduler();
	main_proc = new BerrnM68KProcessor(16000000, inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t param, int64_t)
	{
	    int time_irq = int(param);
	    vblank_callback(time_irq);
	});

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    vblank_start();
	});

	vblank_end_timer = new BerrnTimer("VBlankEnd", scheduler, [&](int64_t, int64_t)
	{
	    vblank_end();
	});
    }

    CaveCore::~CaveCore()
    {

    }

    void CaveCore::init_base(int time_irq)
    {
	interface.init();
	auto &scheduler = driver.get_scheduler();
	scheduler.add_device(main_cpu);
	main_proc->init();
	vblank_timer->start(17376, true, time_irq);
    }

    void CaveCore::stop_base()
    {
	vblank_timer->stop();
    }

    bool CaveCore::init_core()
    {
	return false;
    }

    void CaveCore::stop_core()
    {
	return;
    }

    void CaveCore::run_core()
    {
	driver.run_scheduler();
    }

    void CaveCore::vblank_callback(int time_irq)
    {
	irq_timer->start((17376 - time_irq), false);
    }

    void CaveCore::update_irq_state()
    {
	if (is_vblank_irq)
	{
	    main_proc->fire_interrupt_level(1, true);
	}
	else
	{
	    main_proc->fire_interrupt_level(1, false);
	}
    }

    void CaveCore::vblank_start()
    {
	is_vblank_irq = true;
	update_irq_state();
	vblank_end_timer->start(2000, false);
    }

    void CaveCore::vblank_end()
    {
	return;
    }

    DDonPachiCore::DDonPachiCore(berrndriver &drv) : CaveCore(drv, main_inter), main_inter(drv, *this)
    {

    }

    DDonPachiCore::~DDonPachiCore()
    {

    }

    bool DDonPachiCore::init_core()
    {
	init_base(90);
	driver.resize(240, 320, 2);
	return true;
    }

    void DDonPachiCore::stop_core()
    {
	stop_base();
    }

    driverddonpach::driverddonpach()
    {
	core = new DDonPachiCore(*this);
    }

    driverddonpach::~driverddonpach()
    {

    }

    string driverddonpach::drivername()
    {
	return "ddonpach";
    }

    bool driverddonpach::drvinit()
    {
	if (!loadROM(berrn_rom_name(ddonpach)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverddonpach::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverddonpach::drvrun()
    {
	core->run_core();
    }

    void driverddonpach::keychanged(BerrnInput key, bool is_pressed)
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