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

#include "bombjack.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(bombjack)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("09_j01b.bin", 0x0000, 0x2000)
	    berrn_rom_load("10_l01b.bin", 0x2000, 0x2000)
	    berrn_rom_load("11_m01b.bin", 0x4000, 0x2000)
	    berrn_rom_load("12_n01b.bin", 0x6000, 0x2000)
	    berrn_rom_load("13.1r",       0xC000, 0x2000)
	berrn_rom_region("soundcpu", 0x10000, 0)
	    berrn_rom_load("01_h03t.bin", 0x0000, 0x2000)
	berrn_rom_region("chars", 0x3000, 0)
	    berrn_rom_load("03_e08t.bin", 0x0000, 0x1000)
	    berrn_rom_load("04_h08t.bin", 0x1000, 0x1000)
	    berrn_rom_load("05_k08t.bin", 0x2000, 0x1000)
	berrn_rom_region("tiles", 0x6000, 0)
	    berrn_rom_load("06_l08t.bin", 0x0000, 0x2000)
	    berrn_rom_load("07_n08t.bin", 0x2000, 0x2000)
	    berrn_rom_load("08_r08t.bin", 0x4000, 0x2000)
	berrn_rom_region("sprites", 0x6000, 0)
	    berrn_rom_load("16_m07b.bin", 0x0000, 0x2000)
	    berrn_rom_load("15_l07b.bin", 0x2000, 0x2000)
	    berrn_rom_load("14_j07b.bin", 0x4000, 0x2000)
	berrn_rom_region("bgtilemap", 0x1000, 0)
	    berrn_rom_load("02_p04t.bin", 0x0000, 0x1000)
    berrn_rom_end

    BombJackMain::BombJackMain(berrndriver &drv, BombJackCore &core) : driver(drv), main_core(core)
    {

    }

    BombJackMain::~BombJackMain()
    {

    }

    void BombJackMain::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void BombJackMain::shutdown()
    {
	main_rom.clear();
    }

    uint8_t BombJackMain::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x8000)
	{
	    data = main_rom.at(addr);
	}
	else if (inRange(addr, 0x8000, 0x9000))
	{
	    data = main_ram.at(addr & 0xFFF);
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    data = main_core.readGraphics(0, addr);
	}
	else if (inRange(addr, 0x9400, 0x9800))
	{
	    data = main_core.readGraphics(1, addr);
	}
	else if (addr == 0xB000)
	{
	    // P1
	    data = 0x00;
	}
	else if (addr == 0xB001)
	{
	    // P2
	    data = 0x00;
	}
	else if (addr == 0xB002)
	{
	    // SYSTEM
	    data = 0x00;
	}
	else if (addr == 0xB003)
	{
	    data = 0x00;
	}
	else if (addr == 0xB004)
	{
	    // DSW1
	    data = 0xC0;
	}
	else if (addr == 0xB005)
	{
	    // DSW2
	    data = 0x50;
	}
	else if (inRange(addr, 0xC000, 0xE000))
	{
	    data = main_rom.at(addr);
	}

	return data;
    }

    void BombJackMain::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x8000)
	{
	    return;
	}
	else if (inRange(addr, 0x8000, 0x9000))
	{
	    main_ram.at(addr & 0xFFF) = data;
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    main_core.writeGraphics(0, addr, data);
	}
	else if (inRange(addr, 0x9400, 0x9800))
	{
	    main_core.writeGraphics(1, addr, data);
	}
	else if (inRange(addr, 0x9820, 0x9880))
	{
	    main_core.writeGraphics(2, addr, data);
	}
	else if (addr == 0x9A00)
	{
	    return;
	}
	else if (inRange(addr, 0x9C00, 0x9D00))
	{
	    main_core.writeGraphics(3, addr, data);
	}
	else if (addr == 0x9E00)
	{
	    main_core.writeBackground(data);
	}
	else if (addr == 0xB000)
	{
	    main_core.writeNMI(data);
	}
	else if (addr == 0xB004)
	{
	    // Screen flipping (unimplemented)
	    return;
	}
	else if (addr == 0xB800)
	{
	    main_core.writeSoundLatch(data);
	}
	else if (inRange(addr, 0xC000, 0xE000))
	{
	    return;
	}
    }

    BombJackSound::BombJackSound(berrndriver &drv, BombJackCore &core) : driver(drv), sound_core(core)
    {
	ay1 = new ay8910device(driver);
	ay2 = new ay8910device(driver);
	ay3 = new ay8910device(driver);
    }

    BombJackSound::~BombJackSound()
    {

    }

    void BombJackSound::init()
    {
	ay1->init(1500000);
	ay2->init(1500000);
	ay3->init(1500000);
	sound_rom = driver.get_rom_region("soundcpu");
	sound_ram.fill(0);
    }

    void BombJackSound::shutdown()
    {
	sound_rom.clear();
    }

    uint8_t BombJackSound::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x2000)
	{
	    data = sound_rom.at(addr);
	}
	else if (inRange(addr, 0x4000, 0x4400))
	{
	    data = sound_ram.at((addr & 0x3FF));
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

    void BombJackSound::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x2000)
	{
	    return;
	}
	else if (inRange(addr, 0x4000, 0x4400))
	{
	    sound_ram.at((addr & 0x3FF)) = data;
	}
    }

    void BombJackSound::portOut(uint16_t addr, uint8_t data)
    {
	addr &= 0xFF;

	if (inRangeEx(addr, 0x00, 0x01))
	{
	    int port = addr;
	    ay1->writeIO(port, data);
	}
	else if (inRangeEx(addr, 0x10, 0x11))
	{
	    int port = (addr - 0x10);
	    ay2->writeIO(port, data);
	}
	else if (inRangeEx(addr, 0x80, 0x81))
	{
	    int port = (addr - 0x80);
	    ay3->writeIO(port, data);
	}
    }

    void BombJackSound::processAudio()
    {
	auto ay1_samples = ay1->fetch_samples();
	auto ay2_samples = ay2->fetch_samples();
	auto ay3_samples = ay3->fetch_samples();

	for (auto &psg1_sample : ay1_samples)
	{
	    driver.add_mono_sample(psg1_sample, 0.13);
	}

	for (auto &psg2_sample : ay2_samples)
	{
	    driver.add_mono_sample(psg2_sample, 0.13);
	}

	for (auto &psg3_sample : ay3_samples)
	{
	    driver.add_mono_sample(psg3_sample, 0.13);
	}
    }

    BombJackCore::BombJackCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new BombJackMain(driver, *this);
	main_proc = new BerrnZ80Processor(4000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	sound_inter = new BombJackSound(driver, *this);
	sound_proc = new BerrnZ80Processor(3000000, *sound_inter);
	sound_cpu = new BerrnCPU(scheduler, *sound_proc);

	video = new bombjackvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    if (is_nmi_enabled)
	    {
		main_proc->fire_nmi();
	    }

	    sound_proc->fire_nmi();

	    video->updatePixels();
	});
    }

    BombJackCore::~BombJackCore()
    {

    }

    bool BombJackCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_proc->init();
	sound_inter->init();
	sound_proc->init();
	video->init();
	vblank_timer->start(time_in_hz(60), true);
	scheduler.add_device(main_cpu);
	scheduler.add_device(sound_cpu);
	driver.resize(256, 224, 2);
	return true;
    }

    void BombJackCore::stop_core()
    {
	vblank_timer->stop();
	video->shutdown();
	sound_proc->shutdown();
	sound_inter->shutdown();
	main_proc->shutdown();
	main_inter->shutdown();
    }

    void BombJackCore::run_core()
    {
	driver.run_scheduler();
    }

    void BombJackCore::process_audio()
    {
	sound_inter->processAudio();
    }

    uint8_t BombJackCore::readGraphics(int bank, uint16_t addr)
    {
	uint8_t data = 0;

	switch (bank)
	{
	    case 0: data = video->readVRAM(addr); break;
	    case 1: data = video->readCRAM(addr); break;
	}

	return data;
    }

    void BombJackCore::writeGraphics(int bank, uint16_t addr, uint8_t data)
    {
	switch (bank)
	{
	    case 0: video->writeVRAM(addr, data); break;
	    case 1: video->writeCRAM(addr, data); break;
	    case 2: video->writeORAM(addr, data); break;
	    case 3: video->writePalette(addr, data); break;
	}
    }

    void BombJackCore::writeBackground(uint8_t data)
    {
	video->writeBackground(data);
    }

    void BombJackCore::writeNMI(uint8_t data)
    {
	is_nmi_enabled = testbit(data, 0);

	if (!is_nmi_enabled)
	{
	    main_proc->fire_nmi(false);
	}
    }

    uint8_t BombJackCore::readSoundLatch()
    {
	uint8_t res = sound_cmd;
	sound_cmd = 0x00;
	return res;
    }

    void BombJackCore::writeSoundLatch(uint8_t data)
    {
	sound_cmd = data;
    }

    driverbombjack::driverbombjack()
    {
	core = new BombJackCore(*this);
    }

    driverbombjack::~driverbombjack()
    {

    }

    string driverbombjack::drivername()
    {
	return "bombjack";
    }

    uint32_t driverbombjack::get_flags()
    {
	return berrn_rot_90;
    }

    bool driverbombjack::drvinit()
    {
	if (!loadROM(berrn_rom_name(bombjack)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverbombjack::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverbombjack::drvrun()
    {
	core->run_core();
    }

    void driverbombjack::process_audio()
    {
	core->process_audio();
    }

    void driverbombjack::keychanged(BerrnInput key, bool is_pressed)
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