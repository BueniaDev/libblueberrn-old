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
	    berrn_rom_load("sr-01.c11", 0x0000,  0x4000)
	berrn_rom_region("gfx1", 0x2000, 0)
	    berrn_rom_load("sr-02.f2",  0x0000,  0x2000)
	berrn_rom_region("gfx2", 0xC000, 0)
	    berrn_rom_load("sr-08.a1",  0x0000,  0x2000)
	    berrn_rom_load("sr-09.a2",  0x2000,  0x2000)
	    berrn_rom_load("sr-10.a3",  0x4000,  0x2000)
	    berrn_rom_load("sr-11.a4",  0x6000,  0x2000)
	    berrn_rom_load("sr-12.a5",  0x8000,  0x2000)
	    berrn_rom_load("sr-13.a6",  0xA000,  0x2000)
	berrn_rom_region("gfx3", 0x10000, 0)
	    berrn_rom_load("sr-14.l1",  0x0000,  0x4000)
	    berrn_rom_load("sr-15.l2",  0x4000,  0x4000)
	    berrn_rom_load("sr-16.n1",  0x8000,  0x4000)
	    berrn_rom_load("sr-17.n2",  0xC000,  0x4000)
	berrn_rom_region("palproms",  0x0300, 0)
	    berrn_rom_load("sb-5.e8",   0x0000,  0x0100)
	    berrn_rom_load("sb-6.e9",   0x0100,  0x0100)
	    berrn_rom_load("sb-7.e10",  0x0200,  0x0100)
	berrn_rom_region("charprom",  0x0100, 0)
	    berrn_rom_load("sb-0.f1",   0x0000,  0x0100)
	berrn_rom_region("tileprom",  0x0100, 0)
	    berrn_rom_load("sb-4.d6",   0x0000,  0x0100)
	berrn_rom_region("spriteprom", 0x0100, 0)
	    berrn_rom_load("sb-8.k3",   0x0000,  0x0100)
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
	    data = main_core.readGraphics(2, addr);
	}
	else if (inRange(addr, 0xD000, 0xD800))
	{
	    // Read from foreground RAM
	    data = main_core.readGraphics(1, addr);
	}
	else if (inRange(addr, 0xD800, 0xDC00))
	{
	    // Read from background RAM
	    data = main_core.readGraphics(0, addr);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    data = main_ram.at(addr & 0xFFF);
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void Berrn1942Main::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0xC000)
	{
	    return;
	}
	else if (inRangeEx(addr, 0xC800, 0xC805))
	{
	    main_core.writeIO(addr, data);
	}
	else if (addr == 0xC806)
	{
	    current_rom_bank = (data & 0x3);
	}
	else if (inRange(addr, 0xCC00, 0xCC80))
	{
	    // Write to sprite RAM
	    main_core.writeGraphics(2, addr, data);
	}
	else if (inRange(addr, 0xD000, 0xD800))
	{
	    // Write to foreground RAM
	    main_core.writeGraphics(1, addr, data);
	}
	else if (inRange(addr, 0xD800, 0xDC00))
	{
	    // Write to background RAM
	    main_core.writeGraphics(0, addr, data);
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    main_ram.at(addr & 0xFFF) = data;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
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
	ay1->init(1500000);
	ay2->init(1500000);
	sound_rom = driver.get_rom_region("soundcpu");
	sound_ram.fill(0);
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

    void Berrn1942Sound::process_audio()
    {
	auto ay1_samples = ay1->fetch_samples();
	auto ay2_samples = ay2->fetch_samples();

	for (auto &psg1_sample : ay1_samples)
	{
	    driver.add_mono_sample(psg1_sample);
	}

	for (auto &psg2_sample : ay2_samples)
	{
	    driver.add_mono_sample(psg2_sample);
	}
    }

    Berrn1942Core::Berrn1942Core(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new Berrn1942Main(driver, *this);
	main_proc = new BerrnZ80Processor(3000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	sound_inter = new Berrn1942Sound(driver, *this);
	sound_proc = new BerrnZ80Processor(3000000, *sound_inter);
	sound_cpu = new BerrnCPU(scheduler, *sound_proc);

	video = new berrn1942video(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    video->updatePixels();
	    vblank_start_time = driver.get_scheduler().get_current_time();
	    vblank_timer->start(time_until_pos(246), false);
	});

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    int next_vpos = 0;

	    if (!is_first_time)
	    {
		int current_vpos = vpos();

		scanline_callback(current_vpos);

		if ((current_vpos + 1) < 262)
		{
		    next_vpos = (current_vpos + 1);
		}
	    }

	    is_first_time = false;
	    irq_timer->start(time_until_pos(next_vpos), false);
	});
    }

    Berrn1942Core::~Berrn1942Core()
    {

    }

    void Berrn1942Core::scanline_callback(int scanline)
    {
	if (scanline == 0x2C)
	{
	    sound_proc->fire_interrupt8();
	}

	if (scanline == 0x6D)
	{
	    main_proc->fire_interrupt8(0xCF);
	    sound_proc->fire_interrupt8();
	}

	if (scanline == 0xAF)
	{
	    sound_proc->fire_interrupt8();
	}

	if (scanline == 0xF0)
	{
	    main_proc->fire_interrupt8(0xD7);
	    sound_proc->fire_interrupt8();
	}
    }

    int64_t Berrn1942Core::time_until_pos(int vpos)
    {
	int64_t frame_period = 16768;
	auto &scheduler = driver.get_scheduler();
	vpos = ((vpos + 16) % 262);

	int64_t scan_time = (frame_period / 262);

	int64_t target_delta = (vpos * scan_time);

	int64_t current_delta = (scheduler.get_current_time() - vblank_start_time);

	while (target_delta <= current_delta)
	{
	    target_delta += frame_period;
	}

	return (target_delta - current_delta);
    }

    int Berrn1942Core::vpos()
    {
	int64_t frame_period = 16768;
	auto &scheduler = driver.get_scheduler();

	int64_t delta = (scheduler.get_current_time() - vblank_start_time);

	int64_t pixel_time = (frame_period / (384 * 262));
	int64_t scan_time = (frame_period / 262);

	delta += (pixel_time / 2);

	int vpos = (delta / scan_time);
	return ((vpos + 246) % 262);
    }

    uint8_t Berrn1942Core::readDIP(int reg)
    {
	uint8_t data = 0;
	switch (reg)
	{
	    // SYSTEM
	    case 0:
	    {
		data = 0xFF;
	    }
	    break;
	    // P1
	    case 1:
	    {
		data = p1_port;
	    }
	    break;
	    // P2
	    case 2:
	    {
		data = 0xFF;
	    }
	    break;
	    // DSWA
	    case 3:
	    {
		data = 0x77;
	    }
	    break;
	    // DSWB
	    case 4:
	    {
		// NOTE: The below line enables diagnostic mode
		data = 0xF7;
		// data = 0xFF;
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized DIP read of " << dec << int(reg) << endl;
		exit(0);
	    }
	    break;
	}

	return data;
    }

    void Berrn1942Core::writeC804(uint8_t data)
    {
	// Bit 7 - 1=Screen flipped
	// Bit 4 - Reset line of sound Z80
	// Bit 0 - Coin counter
	sound_cpu->set_reset_line(testbit(data, 4));
    }

    void Berrn1942Core::writeIO(int reg, uint8_t data)
    {
	reg &= 7;
	switch (reg)
	{
	    case 0: writeSoundLatch(data); break;
	    case 2: video->writeScroll(false, data); break;
	    case 3: video->writeScroll(true, data); break;
	    case 4: writeC804(data); break;
	    case 5: video->writePaletteBank(data); break;
	    default: break;
	}
    }

    uint8_t Berrn1942Core::readSoundLatch()
    {
	return sound_cmd;
    }

    void Berrn1942Core::writeSoundLatch(uint8_t data)
    {
	sound_cmd = data;
    }

    uint8_t Berrn1942Core::readGraphics(int bank, uint16_t addr)
    {
	uint8_t data = 0;
	switch (bank)
	{
	    case 0: data = video->readBG(addr); break;
	    case 1: data = video->readFG(addr); break;
	    case 2: data = video->readOBJ(addr); break;
	}

	return data;
    }

    void Berrn1942Core::writeGraphics(int bank, uint16_t addr, uint8_t data)
    {
	switch (bank)
	{
	    case 0: video->writeBG(addr, data); break;
	    case 1: video->writeFG(addr, data); break;
	    case 2: video->writeOBJ(addr, data); break;
	    default: break;
	}
    }

    bool Berrn1942Core::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_proc->init();
	sound_inter->init();
	sound_proc->init();
	video->init();
	p1_port = 0xFF;
	is_first_time = true;
	irq_timer->start(0, false);
	vblank_timer->start(time_until_pos(246), false);
	scheduler.add_device(main_cpu);
	scheduler.add_device(sound_cpu);
	return true;
    }

    void Berrn1942Core::stop_core()
    {
	irq_timer->stop();
	vblank_timer->stop();
	video->shutdown();
	sound_proc->shutdown();
	sound_inter->shutdown();
	main_proc->shutdown();
	main_inter->shutdown();
    }

    void Berrn1942Core::run_core()
    {
	driver.run_scheduler();
    }

    void Berrn1942Core::process_audio()
    {
	sound_inter->process_audio();
    }

    void Berrn1942Core::key_changed(BerrnInput key, bool is_pressed)
    {
	string key_state = (is_pressed) ? "pressed" : "released";

	switch (key)
	{
	    case BerrnInput::BerrnCoin: break;
	    case BerrnInput::BerrnStartP1: break;
	    case BerrnInput::BerrnLeftP1:
	    {
		p1_port = changebit(p1_port, 1, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		p1_port = changebit(p1_port, 0, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnUpP1:
	    {
		p1_port = changebit(p1_port, 3, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnDownP1:
	    {
		p1_port = changebit(p1_port, 2, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnButton1P1:
	    {
		p1_port = changebit(p1_port, 4, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnButton2P1:
	    {
		p1_port = changebit(p1_port, 5, !is_pressed);
	    }
	    break;
	    default: break;
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

    void driver1942::process_audio()
    {
	core->process_audio();
    }

    void driver1942::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }
};