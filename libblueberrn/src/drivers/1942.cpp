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

#include "1942.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(1942)
	berrn_rom_region("maincpu", 0x20000)
	    berrn_rom_load("srb-03.m3", 0x00000, 0x4000)
	    berrn_rom_load("srb-04.m4", 0x04000, 0x4000)
	    berrn_rom_load("srb-05.m5", 0x10000, 0x4000)
	    berrn_rom_load("srb-06.m6", 0x14000, 0x2000)
	    berrn_rom_load("srb-07.m7", 0x18000, 0x4000)
	berrn_rom_region("soundcpu", 0x10000)
	    berrn_rom_load("sr-01.c11", 0x0000, 0x4000)
	berrn_rom_region("gfx1", 0x2000)
	    berrn_rom_load("sr-02.f2", 0x0000, 0x2000)
	berrn_rom_region("gfx2", 0xC000)
	    berrn_rom_load("sr-08.a1", 0x0000, 0x2000)
	    berrn_rom_load("sr-09.a2", 0x2000, 0x2000)
	    berrn_rom_load("sr-10.a3", 0x4000, 0x2000)
	    berrn_rom_load("sr-11.a4", 0x6000, 0x2000)
	    berrn_rom_load("sr-12.a5", 0x8000, 0x2000)
	    berrn_rom_load("sr-13.a6", 0xA000, 0x2000)
	berrn_rom_region("palproms", 0x300)
	    berrn_rom_load("sb-5.e8", 0x0000, 0x0100) // Red component
	    berrn_rom_load("sb-6.e9", 0x0100, 0x0100) // Green component
	    berrn_rom_load("sb-7.e10", 0x0200, 0x0100) // Blue component
	berrn_rom_region("charprom", 0x100)
	    berrn_rom_load("sb-0.f1", 0x0000, 0x0100)
	berrn_rom_region("tileprom", 0x100)
	    berrn_rom_load("sb-4.d6", 0x0000, 0x0100)
    berrn_rom_end

    Berrn1942Main::Berrn1942Main(berrndriver &drv, Berrn1942Core &core) : driver(drv), parent_core(core)
    {

    }

    Berrn1942Main::~Berrn1942Main()
    {

    }

    bool Berrn1942Main::init()
    {
	cpu_rom = driver.get_rom_region("maincpu");
	cpu_ram.fill(0);

	return true;
    }

    void Berrn1942Main::shutdown()
    {
	cpu_rom.clear();
    }

    uint8_t Berrn1942Main::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void Berrn1942Main::writeCPU8(uint16_t addr, uint8_t data)
    {
	writeByte(addr, data);
    }

    uint8_t Berrn1942Main::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t Berrn1942Main::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	if (inRange(addr, 0, 0x8000))
	{
	    data = cpu_rom.at(addr);
	}
	else if (inRange(addr, 0x8000, 0xC000))
	{
	    uint32_t rom_addr = (0x10000 + ((addr & 0x3FFF) + (current_rom_bank * 0x4000)));
	    data = cpu_rom.at(rom_addr);
	}
	else if (addr == 0xC000)
	{
	    // SYSTEM
	    data = 0xFF;
	}
	else if (addr == 0xC001)
	{
	    // P1
	    data = 0xFF;
	}
	else if (addr == 0xC002)
	{
	    // P2
	    data = 0xFF;
	}
	else if (addr == 0xC003)
	{
	    // DSWA
	    data = 0x77;
	}
	else if (addr == 0xC004)
	{
	    // DSWB
	    data = 0xFF;
	}
	else if (inRange(addr, 0xCC00, 0xCC80))
	{
	    data = parent_core.read_graphics(0, addr);
	}
	else if (inRange(addr, 0xD000, 0xDC00))
	{
	    data = parent_core.read_graphics(1, addr);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    data = cpu_ram.at(addr & 0xFFF);
	}
	else
	{
	    cout << "Reading value from address of " << hex << int(addr) << endl;
	    exit(0);
	    data = 0;
	}

	return data;
    }

    void Berrn1942Main::writeByte(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0, 0xC000))
	{
	    return;
	}
	else if (addr == 0xC800)
	{
	    parent_core.write_sound_latch(data);
	}
	else if (inRange(addr, 0xC802, 0xC804))
	{
	    int bank = (addr == 0xC802) ? 0 : 1;
	    parent_core.write_graphics_IO(bank, data);
	}
	else if (addr == 0xC804)
	{
	    if (testbit(data, 4))
	    {
		cout << "Asserting reset line of sound CPU..." << endl;
		parent_core.set_sound_cpu_reset(true);
	    }
	    else
	    {
		cout << "Clearing reset line of sound CPU..." << endl;
		parent_core.set_sound_cpu_reset(false);
	    }
	}
	else if (addr == 0xC805)
	{
	    parent_core.write_graphics_IO(2, data);
	}
	else if (addr == 0xC806)
	{
	    current_rom_bank = (data & 0x3);
	}
	else if (inRange(addr, 0xCC00, 0xCC80))
	{
	    parent_core.write_graphics(0, addr, data);
	}
	else if (inRange(addr, 0xD000, 0xDC00))
	{
	    parent_core.write_graphics(1, addr, data);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    cpu_ram.at(addr & 0xFFF) = data;
	}
	else
	{
	    cout << "Writing value of " << hex << int(data) << " to address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    Berrn1942Sound::Berrn1942Sound(berrndriver &drv, Berrn1942Core &core) : driver(drv), parent_core(core)
    {

    }

    Berrn1942Sound::~Berrn1942Sound()
    {

    }

    bool Berrn1942Sound::init()
    {
	cpu_rom = driver.get_rom_region("soundcpu");
	cpu_ram.fill(0);
	return true;
    }

    void Berrn1942Sound::shutdown()
    {
	cpu_rom.clear();
    }

    uint8_t Berrn1942Sound::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void Berrn1942Sound::writeCPU8(uint16_t addr, uint8_t data)
    {
	writeByte(addr, data);
    }

    uint8_t Berrn1942Sound::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t Berrn1942Sound::readByte(uint16_t addr)
    {
	uint8_t data = 0;

	if (inRange(addr, 0, 0x4000))
	{
	    data = cpu_rom.at(addr);
	}
	else if (inRange(addr, 0x4000, 0x4800))
	{
	    data = cpu_ram.at((addr & 0x7FF));
	}
	else if (addr == 0x6000)
	{
	    data = parent_core.read_sound_latch();
	}
	else
	{
	    cout << "Reading byte from sound CPU address of " << hex << int(addr) << endl;
	    exit(0);
	}

	return data;
    }

    void Berrn1942Sound::writeByte(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0, 0x4000))
	{
	    return;
	}
	else if (inRange(addr, 0x4000, 0x4800))
	{
	    cpu_ram.at((addr & 0x7FF)) = data;
	}
	else if (addr == 0x8000)
	{
	    // cout << "Setting first AY address to " << hex << int(data & 0xF) << endl;
	    return;
	}
	else if (addr == 0x8001)
	{
	    // cout << "Setting first AY data to " << hex << int(data) << endl;
	    return;
	}
	else if (addr == 0xC000)
	{
	    // cout << "Setting second AY address to " << hex << int(data & 0xF) << endl;
	    return;
	}
	else if (addr == 0xC001)
	{
	    // cout << "Setting second AY data to " << hex << int(data) << endl;
	    return;
	}
	else
	{
	    cout << "Writing byte of " << hex << int(data) << " to sound CPU address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    Berrn1942Core::Berrn1942Core(berrndriver &drv) : driver(drv)
    {
	main_inter = new Berrn1942Main(driver, *this);
	main_proc = new BerrnZ80Processor(4000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	sound_inter = new Berrn1942Sound(driver, *this);
	sound_proc = new BerrnZ80Processor(3000000, *sound_inter);
	sound_cpu = new BerrnCPU(scheduler, *sound_proc);

	video_core = new berrn1942video(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    video_core->update_pixels();
	});

	interrupt_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    current_scanline += 1;

	    if (current_scanline == 0x2C)
	    {
		sound_proc->fire_interrupt8(0xFF);
	    }

	    if (current_scanline == 0x6D)
	    {
		main_proc->fire_interrupt8(0xCF);
		sound_proc->fire_interrupt8(0xFF);
	    }

	    if (current_scanline == 0xAF)
	    {
		sound_proc->fire_interrupt8(0xFF);
	    }

	    if (current_scanline == 0xF0)
	    {
		main_proc->fire_interrupt8(0xD7);
		sound_proc->fire_interrupt8(0xFF);
	    }

	    if (current_scanline == 262)
	    {
		current_scanline = 0;
	    }
	});
    }

    Berrn1942Core::~Berrn1942Core()
    {

    }

    bool Berrn1942Core::init_core()
    {
	scheduler.reset();
	scheduler.add_device(main_cpu);
	scheduler.add_device(sound_cpu);
	vblank_timer->start(16768, true);
	interrupt_timer->start(64, true);
	main_inter->init();
	sound_inter->init();
	main_proc->init();
	sound_proc->init();
	video_core->init();
	driver.resize(224, 256, 2);
	return true;
    }

    void Berrn1942Core::shutdown_core()
    {
	video_core->shutdown();
	sound_proc->shutdown();
	main_proc->shutdown();
	sound_inter->shutdown();
	main_inter->shutdown();
	vblank_timer->stop();
	interrupt_timer->stop();
	scheduler.shutdown();
    }

    void Berrn1942Core::run_core()
    {
	int64_t schedule_time = scheduler.get_current_time();
	int64_t frame_time = 16768;

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}
    }

    void Berrn1942Core::key_changed(BerrnInput key, bool is_pressed)
    {
	(void)key;
	(void)is_pressed;
	return;
    }

    uint8_t Berrn1942Core::read_sound_latch()
    {
	return sound_latch;
    }

    void Berrn1942Core::write_sound_latch(uint8_t data)
    {
	sound_latch = data;
    }

    uint8_t Berrn1942Core::read_graphics(int bank, uint16_t addr)
    {
	uint8_t data = 0;

	switch (bank)
	{
	    case 0: data = video_core->readSprites(addr); break;
	    case 1: data = video_core->readByte(addr); break;
	    default: break;
	}

	return data;
    }

    void Berrn1942Core::write_graphics(int bank, uint16_t addr, uint8_t data)
    {
	switch (bank)
	{
	    case 0: video_core->writeSprites(addr, data); break;
	    case 1: video_core->writeByte(addr, data); break;
	    default: break;
	}
    }

    void Berrn1942Core::write_graphics_IO(int bank, uint8_t data)
    {
	switch (bank)
	{
	    case 0: video_core->setScroll(false, data); break;
	    case 1: video_core->setScroll(true, data); break;
	    case 2: video_core->setPaletteBank(data); break;
	    default: break;
	}
    }

    void Berrn1942Core::set_sound_cpu_reset(bool line)
    {
	if (line == true)
	{
	    sound_cpu->suspend(SuspendReason::Reset);
	}
	else
	{
	    if (sound_cpu->is_suspended(SuspendReason::Reset))
	    {
		sound_cpu->reset();
		sound_cpu->resume(SuspendReason::Reset);
	    }
	}
    }

    driver1942::driver1942()
    {
	core = new Berrn1942Core(*this);
    }

    driver1942::~driver1942()
    {

    }

    string driver1942::drivername()
    {
	return "1942";
    }

    bool driver1942::hasdriverROMs()
    {
	return true;
    }

    bool driver1942::drvinit()
    {
	if (!loadROM(berrn_rom_name(1942)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driver1942::drvshutdown()
    {
	core->shutdown_core();
    }
  
    void driver1942::drvrun()
    {
	core->run_core();
    }

    void driver1942::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }
};