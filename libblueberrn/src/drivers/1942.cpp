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
	berrn_rom_region("palproms", 0x0300, 0)
	    berrn_rom_load("sb-5.e8", 0x0000, 0x0100)
	    berrn_rom_load("sb-6.e9", 0x0100, 0x0100)
	    berrn_rom_load("sb-7.e10", 0x0200, 0x0100)
	berrn_rom_region("charprom", 0x0100, 0)
	    berrn_rom_load("sb-0.f1", 0x0000, 0x0100)
	berrn_rom_region("tileprom", 0x0100, 0)
	    berrn_rom_load("sb-4.d6", 0x0000, 0x0100)
	berrn_rom_region("spriteprom", 0x0100, 0)
	    berrn_rom_load("sb-8.k3", 0x0000, 0x0100)
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
	    data = main_core.readDIP((addr - 0xC000));
	}
	else if (inRange(addr, 0xCC00, 0xCC80))
	{
	    // Read from sprite RAM
	    data = main_core.readGraphics(0, addr);
	}
	else if (inRange(addr, 0xD000, 0xD800))
	{
	    // Read from foreground RAM
	    data = main_core.readGraphics(1, addr);
	}
	else if (inRange(addr, 0xD800, 0xDC00))
	{
	    // Read from background RAM
	    data = main_core.readGraphics(2, addr);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    data = main_ram.at((addr - 0xE000));
	}
	else
	{
	    data = 0x00;
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
	else if (inRangeEx(addr, 0xC802, 0xC803))
	{
	    int bank = (addr == 0xC802) ? 0 : 1;
	    main_core.writeGraphicsIO(bank, data);
	}
	else if (addr == 0xC804)
	{
	    main_core.writeC804(data);
	}
	else if (addr == 0xC805)
	{
	    main_core.writeGraphicsIO(2, data);
	}
	else if (addr == 0xC806)
	{
	    current_rom_bank = (data & 0x3);
	}
	else if (inRange(addr, 0xCC00, 0xCC80))
	{
	    // Write to sprite RAM
	    main_core.writeGraphics(0, addr, data);
	}
	else if (inRange(addr, 0xD000, 0xD800))
	{
	    // Write to foreground RAM
	    main_core.writeGraphics(1, addr, data);
	}
	else if (inRange(addr, 0xD800, 0xDC00))
	{
	    // Write to background RAM
	    main_core.writeGraphics(2, addr, data);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    main_ram.at((addr - 0xE000)) = data;
	}
	else
	{
	    return;
	}
    }

    Berrn1942Sound::Berrn1942Sound(berrndriver &drv, Berrn1942Core &core) : driver(drv), main_core(core)
    {
	first_psg = new ay8910device(driver);
	second_psg = new ay8910device(driver);
    }

    Berrn1942Sound::~Berrn1942Sound()
    {

    }

    void Berrn1942Sound::init()
    {
	main_rom = driver.get_rom_region("soundcpu");
	main_ram.fill(0);
	first_psg->init(1500000);
	second_psg->init(1500000);
    }

    void Berrn1942Sound::shutdown()
    {
	first_psg->shutdown();
	second_psg->shutdown();
	main_rom.clear();
    }

    uint8_t Berrn1942Sound::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x4000)
	{
	    data = main_rom.at(addr);
	}
	else if (inRange(addr, 0x4000, 0x4800))
	{
	    data = main_ram.at(addr - 0x4000);
	}
	else if (addr == 0x6000)
	{
	    data = main_core.readSoundLatch();
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
	    main_ram.at(addr - 0x4000) = data;
	}
	else if (addr == 0x8000)
	{
	    first_psg->writeIO(0, data);
	}
	else if (addr == 0x8001)
	{
	    first_psg->writeIO(1, data);
	}
	else if (addr == 0xC000)
	{
	    second_psg->writeIO(0, data);
	}
	else if (addr == 0xC001)
	{
	    second_psg->writeIO(1, data);
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    vector<int32_t> Berrn1942Sound::fetch_samples(bool is_second_psg)
    {
	if (is_second_psg)
	{
	    return second_psg->fetch_samples();
	}
	else
	{
	    return first_psg->fetch_samples();
	}
    }

    Berrn1942Core::Berrn1942Core(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new Berrn1942Main(driver, *this);
	main_proc = new BerrnZ80Processor(4000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	sound_inter = new Berrn1942Sound(driver, *this);
	sound_proc = new BerrnZ80Processor(3000000, *sound_inter);
	sound_cpu = new BerrnCPU(scheduler, *sound_proc);

	video_gfx = new berrn1942video(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    video_gfx->updatePixels();
	});

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    current_scanline += 1;

	    if (current_scanline == 0x2C)
	    {
		sound_proc->fire_interrupt8();
	    }

	    if (current_scanline == 0x6D)
	    {
		main_proc->fire_interrupt8(0xCF); // RST 08H
		sound_proc->fire_interrupt8();
	    }

	    if (current_scanline == 0xAF)
	    {
		sound_proc->fire_interrupt8();
	    }

	    if (current_scanline == 0xF0)
	    {
		main_proc->fire_interrupt8(0xD7); // RST 10H
		sound_proc->fire_interrupt8();
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
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	sound_inter->init();
	main_proc->init();
	sound_proc->init();
	video_gfx->init();
	scheduler.add_device(main_cpu);
	scheduler.add_device(sound_cpu);
	vblank_timer->start(16768, true);
	irq_timer->start(64, true);
	driver.resize(256, 224, 2);
	return true;
    }

    void Berrn1942Core::stop_core()
    {
	vblank_timer->stop();
	irq_timer->stop();
	video_gfx->shutdown();
	sound_proc->shutdown();
	main_proc->shutdown();
	sound_inter->shutdown();
	main_inter->shutdown();
    }

    void Berrn1942Core::run_core()
    {
	driver.run_scheduler();
    }

    void Berrn1942Core::process_audio()
    {
	auto samples_psg1 = sound_inter->fetch_samples(false);

	for (auto &sample : samples_psg1)
	{
	    driver.add_mono_sample(sample);
	}

	auto samples_psg2 = sound_inter->fetch_samples(true);

	for (auto &sample : samples_psg2)
	{
	    driver.add_mono_sample(sample);
	}
    }

    uint8_t Berrn1942Core::readDIP(int addr)
    {
	uint8_t data = 0;
	switch (addr)
	{
	    case 0:
	    {
		// SYSTEM
		data = 0xFF;
	    }
	    break;
	    case 1:
	    {
		// P1
		data = 0xFF;
	    }
	    break;
	    case 2:
	    {
		// P2
		data = 0xFF;
	    }
	    break;
	    case 3:
	    {
		// DSWA
		data = 0x77;
	    }
	    break;
	    case 4:
	    {
		// DSWB (Note: Return 0xF7 here to activate 'diagnostic' mode)
		data = 0xFF;
		// data = 0xF7;
	    }
	    break;
	    default: break;
	}

	return data;
    }

    uint8_t Berrn1942Core::readSoundLatch()
    {
	return sound_cmd;
    }

    void Berrn1942Core::writeSoundLatch(uint8_t data)
    {
	sound_cmd = data;
    }

    void Berrn1942Core::writeC804(uint8_t data)
    {
	sound_cpu->set_reset_line(testbit(data, 4));
    }

    uint8_t Berrn1942Core::readGraphics(int bank, uint16_t addr)
    {
	uint8_t data = 0;

	switch (bank)
	{
	    case 0: data = video_gfx->readOBJ(addr); break;
	    case 1: data = video_gfx->readFG(addr); break;
	    case 2: data = video_gfx->readBG(addr); break;
	}

	return data;
    }

    void Berrn1942Core::writeGraphics(int bank, uint16_t addr, uint8_t data)
    {
	switch (bank)
	{
	    case 0: video_gfx->writeOBJ(addr, data); break;
	    case 1: video_gfx->writeFG(addr, data); break;
	    case 2: video_gfx->writeBG(addr, data); break;
	}
    }

    void Berrn1942Core::writeGraphicsIO(int bank, uint8_t data)
    {
	switch (bank)
	{
	    case 0: video_gfx->setScroll(false, data); break;
	    case 1: video_gfx->setScroll(true, data); break;
	    case 2: video_gfx->setPaletteBank(data); break;
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

    uint32_t driver1942::get_flags()
    {
	return berrn_rot_270;
    }

    double driver1942::get_framerate()
    {
	return (1e6 / 16768);
    }

    void driver1942::process_audio()
    {
	core->process_audio();
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