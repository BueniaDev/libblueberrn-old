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

// 1942 - Capcom (WIP)
//
// TODO:
// Implement sound system

namespace berrn
{
    berrn_rom_start(1942)
	berrn_rom_region("maincpu", 0x20000, berrn_rom_eraseff)
	    berrn_rom_load("srb-03.m3", 0x00000, 0x4000)
	    berrn_rom_load("srb-04.m4", 0x04000, 0x4000)
	    berrn_rom_load("srb-05.m5", 0x10000, 0x4000)
	    berrn_rom_load("srb-06.m6", 0x14000, 0x2000)
	    berrn_rom_load("srb-07.m7", 0x18000, 0x4000)
	berrn_rom_region("soundcpu", 0x10000, 0)
	    berrn_rom_load("sr-01.c11", 0x0000, 0x4000)
	berrn_rom_region("gfx1", 0x2000, 0)
	    berrn_rom_load("sr-02.f2", 0x0000, 0x2000)
	berrn_rom_region("gfx2", 0xC000, 0)
	    berrn_rom_load("sr-08.a1", 0x0000, 0x2000)
	    berrn_rom_load("sr-09.a2", 0x2000, 0x2000)
	    berrn_rom_load("sr-10.a3", 0x4000, 0x2000)
	    berrn_rom_load("sr-11.a4", 0x6000, 0x2000)
	    berrn_rom_load("sr-12.a5", 0x8000, 0x2000)
	    berrn_rom_load("sr-13.a6", 0xA000, 0x2000)
	berrn_rom_region("gfx3", 0x10000, 0)
	    berrn_rom_load("sr-14.l1", 0x0000, 0x4000)
	    berrn_rom_load("sr-15.l2", 0x4000, 0x4000)
	    berrn_rom_load("sr-16.n1", 0x8000, 0x4000)
	    berrn_rom_load("sr-17.n2", 0xC000, 0x4000)
	berrn_rom_region("colorproms", 0x0300, 0)
	    berrn_rom_load("sb-5.e8",  0x0000, 0x0100)
	    berrn_rom_load("sb-6.e9",  0x0100, 0x0100)
	    berrn_rom_load("sb-7.e10", 0x0200, 0x0100)
	berrn_rom_region("charprom", 0x0100, 0)
	    berrn_rom_load("sb-0.f1",  0x0000, 0x0100)
	berrn_rom_region("tileprom", 0x0100, 0)
	    berrn_rom_load("sb-4.d6",  0x0000, 0x0100)
	berrn_rom_region("objprom",  0x0100, 0)
	    berrn_rom_load("sb-8.k3",  0x0000, 0x0100)
    berrn_rom_end

    Berrn1942Main::Berrn1942Main(berrndriver &drv, Berrn1942Core &core) : driver(drv), main_core(core)
    {

    }

    Berrn1942Main::~Berrn1942Main()
    {

    }

    void Berrn1942Main::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void Berrn1942Main::shutdown()
    {
	main_rom.clear();
    }

    uint8_t Berrn1942Main::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x8000)
	{
	    data = main_rom.at(addr);
	}
	else if (inRange(addr, 0x8000, 0xC000))
	{
	    uint32_t rom_addr = (0x10000 + ((addr - 0x8000) + (current_rom_bank * 0x4000)));
	    data = main_rom.at(rom_addr);
	}
	else if (inRangeEx(addr, 0xC000, 0xC004))
	{
	    int reg = (addr - 0xC000);
	    data = main_core.readDIP(reg);
	}
	else if (inRange(addr, 0xCC00, 0xCC80))
	{
	    data = main_core.readGraphics(0, addr);
	}
	else if (inRange(addr, 0xD000, 0xD800))
	{
	    data = main_core.readGraphics(1, addr);
	}
	else if (inRange(addr, 0xD800, 0xDC00))
	{
	    data = main_core.readGraphics(2, addr);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    data = main_ram.at(addr & 0xFFF);
	}

	return data;
    }

    void Berrn1942Main::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0xC000)
	{
	    return;
	}
	else if (addr == 0xC800)
	{
	    main_core.writeSoundLatch(data);
	}
	else if (addr == 0xC802)
	{
	    main_core.writeIO(0, data);
	}
	else if (addr == 0xC803)
	{
	    main_core.writeIO(1, data);
	}
	else if (addr == 0xC804)
	{
	    main_core.writeIO(2, data);
	}
	else if (addr == 0xC805)
	{
	    main_core.writeIO(3, data);
	}
	else if (addr == 0xC806)
	{
	    current_rom_bank = (data & 0x3);
	}
	else if (inRange(addr, 0xCC00, 0xCC80))
	{
	    main_core.writeGraphics(0, addr, data);
	}
	else if (inRange(addr, 0xD000, 0xD800))
	{
	    main_core.writeGraphics(1, addr, data);
	}
	else if (inRange(addr, 0xD800, 0xDC00))
	{
	    main_core.writeGraphics(2, addr, data);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    main_ram.at(addr & 0xFFF) = data;
	}
    }

    Berrn1942Sound::Berrn1942Sound(berrndriver &drv, Berrn1942Core &core) : driver(drv), sound_core(core)
    {
	ay1 = new ay8910device(driver);
	ay2 = new ay8910device(driver);
    }

    Berrn1942Sound::~Berrn1942Sound()
    {

    }

    void Berrn1942Sound::init()
    {
	sound_rom = driver.get_rom_region("soundcpu");
	sound_ram.fill(0);
	ay1->init(1500000);
	ay2->init(1500000);
    }

    void Berrn1942Sound::shutdown()
    {
	sound_rom.clear();
    }

    uint8_t Berrn1942Sound::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x4000)
	{
	    data = sound_rom.at(addr);
	}
	else if (inRange(addr, 0x4000, 0x4800))
	{
	    data = sound_ram.at(addr & 0x7FF);
	}
	else if (addr == 0x6000)
	{
	    data = sound_core.readSoundLatch();
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void Berrn1942Sound::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x4000)
	{
	    return;
	}
	else if (inRange(addr, 0x4000, 0x4800))
	{
	    sound_ram.at(addr & 0x7FF) = data;
	}
	else if (inRangeEx(addr, 0x8000, 0x8001))
	{
	    int port = (addr - 0x8000);
	    ay1->writeIO(port, data);
	}
	else if (inRangeEx(addr, 0xC000, 0xC001))
	{
	    int port = (addr - 0xC000);
	    ay2->writeIO(port, data);
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    void Berrn1942Sound::processAudio()
    {
	auto ay1_samples = ay1->fetch_samples();
	auto ay2_samples = ay2->fetch_samples();

	for (auto &sample : ay1_samples)
	{
	    driver.add_mono_sample(sample);
	}

	for (auto &sample : ay2_samples)
	{
	    driver.add_mono_sample(sample);
	}
    }

    Berrn1942Core::Berrn1942Core(berrndriver &drv) : driver(drv)
    {
	main_inter = new Berrn1942Main(driver, *this);
	main_cpu = new BerrnZ80CPU(driver, 3000000, *main_inter);

	sound_inter = new Berrn1942Sound(driver, *this);
	sound_cpu = new BerrnZ80CPU(driver, 3000000, *sound_inter);

	timer = new berrnscanlinetimer(driver);

	video = new berrn1942video(driver);
    }

    Berrn1942Core::~Berrn1942Core()
    {

    }

    bool Berrn1942Core::init_core()
    {
	auto screen = driver.get_screen();
	screen->set_raw(6000000, 384, 128, 0, 262, 22, 246);
	screen->set_vblank_callback([&](bool line) -> void
	{
	    if (!line)
	    {
		return;
	    }

	    video->updatePixels();
	});

	timer->configure([&](int vpos)
	{
	    scanline_callback(vpos);
	}, 0, 1);

	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_cpu->init();
	sound_inter->init();
	sound_cpu->init();
	scheduler.add_device(main_cpu);
	scheduler.add_device(sound_cpu);
	screen->init();
	timer->init();
	video->init();
	system_reg = 0xFF;
	p1_reg = 0xFF;
	driver.resize(256, 224, 2);
	return true;
    }

    void Berrn1942Core::stop_core()
    {
	auto screen = driver.get_screen();
	video->shutdown();
	sound_cpu->shutdown();
	sound_inter->shutdown();
	main_cpu->shutdown();
	main_inter->shutdown();
	timer->shutdown();
	screen->shutdown();
    }

    void Berrn1942Core::run_core()
    {
	driver.run_scheduler();
    }

    void Berrn1942Core::key_changed(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		system_reg = changebit(system_reg, 7, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		system_reg = changebit(system_reg, 0, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		p1_reg = changebit(p1_reg, 1, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		p1_reg = changebit(p1_reg, 0, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnUpP1:
	    {
		p1_reg = changebit(p1_reg, 3, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnDownP1:
	    {
		p1_reg = changebit(p1_reg, 2, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnButton1P1:
	    {
		p1_reg = changebit(p1_reg, 4, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnButton2P1:
	    {
		p1_reg = changebit(p1_reg, 5, !is_pressed);
	    }
	    break;
	    default: break;
	}
    }

    uint8_t Berrn1942Core::readGraphics(int bank, uint16_t addr)
    {
	uint8_t data = 0;
	switch (bank)
	{
	    case 0: data = video->readOBJ(addr); break;
	    case 1: data = video->readFG(addr); break;
	    case 2: data = video->readBG(addr); break;
	}

	return data;
    }

    void Berrn1942Core::writeGraphics(int bank, uint16_t addr, uint8_t data)
    {
	switch (bank)
	{
	    case 0: video->writeOBJ(addr, data); break;
	    case 1: video->writeFG(addr, data); break;
	    case 2: video->writeBG(addr, data); break;
	}
    }

    void Berrn1942Core::writeIO(int addr, uint8_t data)
    {
	switch (addr)
	{
	    case 0: video->writeScroll(false, data); break;
	    case 1: video->writeScroll(true, data); break;
	    case 2: writeC804(data); break;
	    case 3: video->writePaletteBank(data); break;
	}
    }

    void Berrn1942Core::writeC804(uint8_t data)
    {
	// Bit 7 - Flip screen (1=Flipped)
	// Bit 4 - Sound CPU reset
	// Bit 0 - Coin counter
	sound_cpu->set_reset_line(testbit(data, 4));
    }

    uint8_t Berrn1942Core::readSoundLatch()
    {
	return sound_cmd;
    }

    void Berrn1942Core::writeSoundLatch(uint8_t data)
    {
	sound_cmd = data;
    }

    void Berrn1942Core::scanline_callback(int vpos)
    {
	int scanline = vpos;

	if (scanline == 0x2C)
	{
	    sound_cpu->fireInterrupt8();
	}

	if (scanline == 0x6D)
	{
	    main_cpu->fireInterrupt8(0xCF);
	    sound_cpu->fireInterrupt8();
	}

	if (scanline == 0xAF)
	{
	    sound_cpu->fireInterrupt8();
	}

	if (scanline == 0xF0)
	{
	    main_cpu->fireInterrupt8(0xD7);
	    sound_cpu->fireInterrupt8();
	}
    }

    uint8_t Berrn1942Core::readDIP(int reg)
    {
	uint8_t data = 0;
	switch (reg)
	{
	    case 0: data = system_reg;; break; // SYSTEM
	    case 1: data = p1_reg; break; // P1
	    case 2: data = 0xFF; break; // P2
	    case 3: data = 0x77; break; // DSWA
	    case 4: data = 0xFF; break; // DSWB
	    // case 4: data = 0xF7; break; // DSWB
	}

	return data;
    }

    void Berrn1942Core::process_audio()
    {
	sound_inter->processAudio();
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

    uint32_t driver1942::get_flags()
    {
	return berrn_rot_270;
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
	core->stop_core();
    }
  
    void driver1942::drvrun()
    {
	core->run_core();
    }

    void driver1942::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }

    void driver1942::process_audio()
    {
	core->process_audio();
    }
};