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

#include "mia.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(mia)
	berrn_rom_region("maincpu", 0x40000, 0)
	    berrn_rom_load16_byte("808t20.h17", 0x00000, 0x20000)
	    berrn_rom_load16_byte("808t21.j17", 0x00001, 0x20000)
	berrn_rom_region("soundcpu", 0x10000, 0)
	    berrn_rom_load("808e03.f4", 0x0000, 0x8000)
	berrn_rom_region("k052109", 0x40000, 0)
	    berrn_rom_load32_byte("808e12.f28", 0x00000, 0x10000)
	    berrn_rom_load32_byte("808e13.h28", 0x00001, 0x10000)
	    berrn_rom_load32_byte("808e22.i28", 0x00002, 0x10000)
	    berrn_rom_load32_byte("808e23.k28", 0x00003, 0x10000)
	berrn_rom_region("k051960", 0x100000, 0)
	    berrn_rom_load32_word("808d17.j4", 0x00000, 0x80000)
	    berrn_rom_load32_word("808d15.h4", 0x00002, 0x80000)
	berrn_rom_region("k007232", 0x20000, 0)
	    berrn_rom_load("808d01.d4", 0x00000, 0x20000)
	// TODO: Implement LLE priority encoding
	berrn_rom_region("priprom", 0x0100, 0)
	    berrn_rom_load("808a18.f16", 0x0000, 0x0100)
    berrn_rom_end

    MIAM68K::MIAM68K(berrndriver &drv, MIACore &core) : driver(drv), main_core(core)
    {

    }

    MIAM68K::~MIAM68K()
    {

    }

    void MIAM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram[0].fill(0);
	main_ram[1].fill(0);
    }

    void MIAM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t MIAM68K::readCPU16(bool upper, bool lower, uint32_t addr)
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
	else if (inRange(addr, 0x40000, 0x44000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);

	    if (upper)
	    {
		data |= (main_ram[0].at(ram_addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_ram[0].at(ram_addr + 1);
	    }
	}
	else if (inRange(addr, 0x60000, 0x64000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);

	    if (upper)
	    {
		data |= (main_ram[1].at(ram_addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_ram[1].at(ram_addr + 1);
	    }
	}
	else if (inRange(addr, 0x80000, 0x81000))
	{
	    if (lower)
	    {
		data = main_core.readPalette(addr);
	    }
	}
	else if (addr == 0xA0000)
	{
	    // COINS
	    if (lower)
	    {
		data = main_core.readDIP(0);
	    }
	}
	else if (addr == 0xA0002)
	{
	    // P1
	    if (lower)
	    {
		data = main_core.readDIP(1);
	    }
	}
	else if (addr == 0xA0004)
	{
	    // P2
	    if (lower)
	    {
		data = main_core.readDIP(2);
	    }
	}
	else if (addr == 0xA0010)
	{
	    // DSW1
	    if (lower)
	    {
		data = main_core.readDIP(3);
	    }
	}
	else if (addr == 0xA0012)
	{
	    // DSW2
	    if (lower)
	    {
		data = main_core.readDIP(4);
	    }
	}
	else if (addr == 0xA0018)
	{
	    // DSW3
	    if (lower)
	    {
		data = main_core.readDIP(5);
	    }
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    data = main_core.readk052109_noA12(upper, lower, addr);
	}
	else if (inRange(addr, 0x140000, 0x140800))
	{
	    uint32_t sprite_addr = (addr & 0x7FE);

	    if (upper)
	    {
		data |= (main_core.k051960_read(sprite_addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_core.k051960_read(sprite_addr + 1);
	    }
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    void MIAM68K::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x40000)
	{
	    return;
	}
	else if (inRange(addr, 0x40000, 0x44000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);

	    if (upper)
	    {
		main_ram[0].at(ram_addr) = (data >> 8);
	    }

	    if (lower)
	    {
		main_ram[0].at(ram_addr + 1) = (data & 0xFF);
	    }
	}
	else if (inRange(addr, 0x60000, 0x64000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);

	    if (upper)
	    {
		main_ram[1].at(ram_addr) = (data >> 8);
	    }

	    if (lower)
	    {
		main_ram[1].at(ram_addr + 1) = (data & 0xFF);
	    }
	}
	else if (inRange(addr, 0x80000, 0x81000))
	{
	    if (lower)
	    {
		main_core.writePalette(addr, data);
	    }
	}
	else if (addr == 0xA0000)
	{
	    if (lower)
	    {
		main_core.write0A0000(data);
	    }
	}
	else if (addr == 0xA0008)
	{
	    if (lower)
	    {
		main_core.writeSoundLatch(data);
	    }
	}
	else if (addr == 0xA0010)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (addr == 0xC0000)
	{
	    return;
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    main_core.writek052109_noA12(upper, lower, addr, data);
	}
	else if (addr == 0x10E800)
	{
	    return; // ???
	}
	else if (inRange(addr, 0x140000, 0x140800))
	{
	    uint32_t sprite_addr = (addr & 0x7FE);

	    if (upper)
	    {
		main_core.k051960_write(sprite_addr, (data >> 8));
	    }

	    if (lower)
	    {
		main_core.k051960_write((sprite_addr + 1), (data & 0xFF));
	    }
	}
	else
	{
	    BerrnInterface::writeCPU16(upper, lower, addr, data);
	}
    }

    MIAZ80::MIAZ80(berrndriver &drv, MIACore &core) : driver(drv), sound_core(core)
    {
	opm = new ym2151device(driver);
	k007232 = new k007232device(driver);
    }

    MIAZ80::~MIAZ80()
    {

    }

    void MIAZ80::init()
    {
	opm->init(3579545);
	k007232->init(3579545);

	k007232->setVolumeCallback([&](uint8_t data) -> void
	{
	    k007232->setVolume((data >> 4), (data & 0xF));
	});

	sound_rom = driver.get_rom_region("soundcpu");
	sound_ram.fill(0);
    }

    void MIAZ80::shutdown()
    {
	sound_rom.clear();
    }

    uint8_t MIAZ80::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x8000)
	{
	    data = sound_rom.at(addr);
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    data = sound_ram.at(addr & 0x7FF);
	}
	else if (addr == 0xA000)
	{
	    data = sound_core.readSoundLatch();
	}
	else if (inRangeEx(addr, 0xB000, 0xB00D))
	{
	    int reg = (addr - 0xB000);
	    data = k007232->readReg(reg);
	}
	else if (inRangeEx(addr, 0xC000, 0xC001))
	{
	    int port = (addr - 0xC000);
	    data = opm->readIO(port);
	}

	return data;
    }

    void MIAZ80::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x8000)
	{
	    return;
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    sound_ram.at(addr & 0x7FF) = data;
	}
	else if (inRangeEx(addr, 0xB000, 0xB00D))
	{
	    int reg = (addr - 0xB000);
	    k007232->writeReg(reg, data);
	}
	else if (inRangeEx(addr, 0xC000, 0xC001))
	{
	    int port = (addr - 0xC000);
	    opm->writeIO(port, data);
	}
    }

    void MIAZ80::processAudio()
    {
	auto opm_samples = opm->fetch_samples();
	auto k007232_samples = k007232->fetch_samples();

	for (auto &ym_sample : opm_samples)
	{
	    driver.add_mono_sample(ym_sample);
	}

	for (auto &k007232_sample : k007232_samples)
	{
	    driver.add_mono_sample(k007232_sample, 0.20);
	}
    }

    MIACore::MIACore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new MIAM68K(driver, *this);
	main_cpu = new BerrnM68KCPU(driver, 8000000, *main_inter);

	sound_inter = new MIAZ80(driver, *this);
	sound_cpu = new BerrnZ80CPU(driver, 3579545, *sound_inter);

	video = new miavideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    if (is_irq_enabled)
	    {
		main_cpu->fireInterruptLevel(5);
	    }

	    video->updatePixels();
	});
    }

    MIACore::~MIACore()
    {

    }

    bool MIACore::init_core()
    {
	coins_port = 0xFF;
	p1_port = 0xFF;
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	sound_inter->init();
	main_cpu->init();
	sound_cpu->init();
	video->init();
	vblank_timer->start(time_in_hz(60), true);
	scheduler.add_device(main_cpu);
	scheduler.add_device(sound_cpu);
	driver.resize(304, 224, 2);
	return true;
    }

    void MIACore::stop_core()
    {
	vblank_timer->stop();
	video->shutdown();
	sound_inter->shutdown();
	sound_cpu->shutdown();
	main_inter->shutdown();
	main_cpu->shutdown();
    }

    void MIACore::run_core()
    {
	driver.run_scheduler();
    }

    void MIACore::key_changed(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		coins_port = changebit(coins_port, 0, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		coins_port = changebit(coins_port, 3, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		p1_port = changebit(p1_port, 0, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		p1_port = changebit(p1_port, 1, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnUpP1:
	    {
		p1_port = changebit(p1_port, 2, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnDownP1:
	    {
		p1_port = changebit(p1_port, 3, !is_pressed);
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
	    case BerrnInput::BerrnButton3P1:
	    {
		p1_port = changebit(p1_port, 6, !is_pressed);
	    }
	    break;
	    default: break;
	}
    }

    void MIACore::process_audio()
    {
	sound_inter->processAudio();
    }

    uint16_t MIACore::readk052109_noA12(bool upper, bool lower, uint32_t addr)
    {
	return video->tile_read(upper, lower, addr);
    }

    void MIACore::writek052109_noA12(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	video->tile_write(upper, lower, addr, data);
    }

    uint8_t MIACore::k051960_read(uint16_t addr)
    {
	return video->sprite_read(addr);
    }

    void MIACore::k051960_write(uint16_t addr, uint8_t data)
    {
	video->sprite_write(addr, data);
    }

    uint8_t MIACore::readPalette(uint32_t addr)
    {
	return video->palette_read(addr);
    }

    void MIACore::writePalette(uint32_t addr, uint8_t data)
    {
	video->palette_write(addr, data);
    }

    void MIACore::write0A0000(uint16_t data)
    {
	if (prev_int && !testbit(data, 3))
	{
	    sound_cpu->fireInterrupt8();
	}

	prev_int = testbit(data, 3);
	is_irq_enabled = testbit(data, 5);
	video->setRMRD(testbit(data, 7));
    }

    uint8_t MIACore::readDIP(int reg)
    {
	uint8_t data = 0;
	switch (reg)
	{
	    case 0: data = coins_port; break; // COINS
	    case 1: data = p1_port; break; // P1
	    case 2: data = 0xFF; break; // P2
	    case 3: data = 0xFF; break; // DSW1
	    case 4: data = 0x2E; break; // DSW2
	    case 5: data = 0x0F; break; // DSW3
	    // case 5: data = 0x0B; break; // DSW3
	}

	return data;
    }

    uint8_t MIACore::readSoundLatch()
    {
	return sound_cmd;
    }

    void MIACore::writeSoundLatch(uint8_t data)
    {
	sound_cmd = data;
    }

    drivermia::drivermia()
    {
	core = new MIACore(*this);
    }

    drivermia::~drivermia()
    {

    }

    string drivermia::drivername()
    {
	return "mia";
    }

    bool drivermia::drvinit()
    {
	if (!loadROM(berrn_rom_name(mia)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivermia::drvshutdown()
    {
	core->stop_core();
    }
  
    void drivermia::drvrun()
    {
	core->run_core();
    }

    void drivermia::process_audio()
    {
	core->process_audio();
    }

    void drivermia::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }
};