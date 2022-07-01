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

#include "punkshot.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(punkshot)
	berrn_rom_region("maincpu", 0x40000, 0)
	    berrn_rom_load16_byte("907-j02.i7",  0x00000, 0x20000)
	    berrn_rom_load16_byte("907-j03.i10", 0x00001, 0x20000)
	berrn_rom_region("k052109", 0x80000, 0)
	    berrn_rom_load32_word("907d06.e23",  0x00000, 0x40000)
	    berrn_rom_load32_word("907d05.e22",  0x00002, 0x40000)
	berrn_rom_region("k051960", 0x200000, 0)
	    berrn_rom_load32_word("907d07.k2",   0x00000, 0x100000)
	    berrn_rom_load32_word("907d08.k7",   0x00002, 0x100000)
    berrn_rom_end

    PunkshotM68K::PunkshotM68K(berrndriver &drv, PunkshotCore &core) : driver(drv), main_core(core)
    {

    }

    PunkshotM68K::~PunkshotM68K()
    {

    }

    void PunkshotM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
    }

    void PunkshotM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t PunkshotM68K::readCPU16(bool upper, bool lower, uint32_t addr)
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
	else if (inRange(addr, 0x80000, 0x84000))
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
	else if (inRange(addr, 0x90000, 0x91000))
	{
	    data = main_core.palette_read(upper, lower, addr);
	}
	else if (addr == 0xA0000)
	{
	    // DSW1_DSW2
	    if (upper)
	    {
		data |= 0x7B00;
	    }

	    if (lower)
	    {
		data |= 0xFF;
	    }
	}
	else if (addr == 0xA0002)
	{
	    // COINS_DSW3
	    if (upper)
	    {
		data |= 0xFF00;
	    }

	    if (lower)
	    {
		data |= 0xFF;
	    }
	}
	else if (addr == 0xA0004)
	{
	    // P3_P4
	    if (upper)
	    {
		data |= 0xFF00;
	    }

	    if (lower)
	    {
		data |= 0xFF;
	    }
	}
	else if (addr == 0xA0006)
	{
	    // P1_P2
	    if (upper)
	    {
		data |= 0xFF00;
	    }

	    if (lower)
	    {
		data |= 0xFF;
	    }
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    data = main_core.tile_read(upper, lower, addr);
	}
	else if (inRange(addr, 0x110000, 0x110800))
	{
	    uint32_t sprite_addr = (addr & 0x7FE);

	    if (upper)
	    {
		data |= (main_core.sprite_read(sprite_addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_core.sprite_read(sprite_addr + 1);
	    }
	}
	else if (inRange(addr, 0xFFFFFC, 0x1000000))
	{
	    uint16_t kludge = driver.gen_rand();

	    if (upper)
	    {
		data |= (kludge & 0xFF00);
	    }

	    if (lower)
	    {
		data |= (kludge & 0xFF);
	    }
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    void PunkshotM68K::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x40000)
	{
	    return;
	}
	else if (inRange(addr, 0x80000, 0x84000))
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
	else if (inRange(addr, 0x90000, 0x91000))
	{
	    main_core.palette_write(upper, lower, addr, data);
	}
	else if (addr == 0xA0020)
	{
	    if (lower)
	    {
		main_core.writeA0020(data);
	    }
	}
	else if (inRange(addr, 0xA0040, 0xA0044))
	{
	    // TODO: Add K053260 core
	    return;
	}
	else if (inRange(addr, 0xA0060, 0xA0080))
	{
	    if (lower)
	    {
		main_core.priority_write(addr, data);
	    }
	}
	else if (addr == 0xA0080)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    main_core.tile_write(upper, lower, addr, data);
	}
	else if (addr == 0x10E800)
	{
	    return; // ???
	}
	else if (inRange(addr, 0x110000, 0x110800))
	{
	    uint32_t sprite_addr = (addr & 0x7FE);

	    if (upper)
	    {
		main_core.sprite_write(sprite_addr, (data >> 8));
	    }

	    if (lower)
	    {
		main_core.sprite_write((sprite_addr + 1), (data & 0xFF));
	    }
	}
	else
	{
	    BerrnInterface::writeCPU16(upper, lower, addr, data);
	}
    }

    PunkshotCore::PunkshotCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new PunkshotM68K(driver, *this);
	main_cpu = new BerrnM68KCPU(driver, 12000000, *main_inter);
	video = new punkshotvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    if (video->isIRQEnabled())
	    {
		main_cpu->fireInterruptLevel(4, true);
	    }

	    video->updatePixels();
	});
    }

    PunkshotCore::~PunkshotCore()
    {

    }

    bool PunkshotCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_cpu->init();
	video->init();
	vblank_timer->start(time_in_hz(60), true);
	scheduler.add_device(main_cpu);
	// driver.resize(512, 256, 2);
	driver.resize(288, 224, 2);
	return true;
    }

    void PunkshotCore::stop_core()
    {
	video->shutdown();
	main_inter->shutdown();
	main_cpu->shutdown();
    }

    void PunkshotCore::run_core()
    {
	driver.run_scheduler();
    }

    uint16_t PunkshotCore::tile_read(bool upper, bool lower, uint32_t addr)
    {
	return video->tileRead(upper, lower, addr);
    }

    void PunkshotCore::tile_write(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	video->tileWrite(upper, lower, addr, data);
    }

    void PunkshotCore::priority_write(uint32_t addr, uint8_t data)
    {
	video->priorityWrite(addr, data);
    }

    uint8_t PunkshotCore::sprite_read(uint16_t addr)
    {
	return video->spriteRead(addr);
    }

    void PunkshotCore::sprite_write(uint16_t addr, uint8_t data)
    {
	video->spriteWrite(addr, data);
    }

    uint16_t PunkshotCore::palette_read(bool upper, bool lower, uint32_t addr)
    {
	return video->paletteRead(upper, lower, addr);
    }

    void PunkshotCore::palette_write(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	video->paletteWrite(upper, lower, addr, data);
    }

    void PunkshotCore::writeA0020(uint8_t data)
    {
	string z80_str = testbit(data, 2) ? "Asserting" : "Clearing";
	cout << z80_str << " bit 2..." << endl;
	video->setRMRD(testbit(data, 3));
    }

    driverpunkshot::driverpunkshot()
    {
	core = new PunkshotCore(*this);
    }

    driverpunkshot::~driverpunkshot()
    {

    }

    string driverpunkshot::drivername()
    {
	return "punkshot";
    }

    bool driverpunkshot::drvinit()
    {
	if (!loadROM(berrn_rom_name(punkshot)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverpunkshot::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverpunkshot::drvrun()
    {
	core->run_core();
    }

    void driverpunkshot::keychanged(BerrnInput key, bool is_pressed)
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