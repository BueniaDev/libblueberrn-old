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

#include <tmnt.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    berrn_rom_start(tmnt)
	berrn_rom_region("maincpu", 0x60000, 0)
	    berrn_rom_load16_byte("963-x23.j17", 0x00000, 0x20000)
	    berrn_rom_load16_byte("963-x24.k17", 0x00001, 0x20000)
	    berrn_rom_load16_byte("963-x21.j15", 0x40000, 0x10000)
	    berrn_rom_load16_byte("963-x22.k15", 0x40001, 0x10000)
	berrn_rom_region("soundcpu", 0x10000, 0)
	    berrn_rom_load("963e20.g13", 0x0000, 0x8000)
	berrn_rom_region("k052109", 0x100000, 0)
	    berrn_rom_load32_word("963a28.h27", 0x000000, 0x80000)
	    berrn_rom_load32_word("963a29.k27", 0x000002, 0x80000)
	berrn_rom_region("k051960", 0x200000, 0)
	    berrn_rom_load32_word("963a17.h4", 0x000000, 0x80000)
	    berrn_rom_load32_word("963a15.k4", 0x000002, 0x80000)
	    berrn_rom_load32_word("963a18.h6", 0x100000, 0x80000)
	    berrn_rom_load32_word("963a16.k6", 0x100002, 0x80000)
	berrn_rom_region("sprprom", 0x0100, 0)
	    berrn_rom_load("963a30.g7", 0x0000, 0x0100)
	berrn_rom_region("k007232", 0x20000, 0)
	    berrn_rom_load("963a26.c13", 0x00000, 0x20000)
	berrn_rom_region("title", 0x80000, 0)
	    berrn_rom_load("963a25.d5", 0x00000, 0x80000)
    berrn_rom_end

    TMNTM68K::TMNTM68K(berrndriver &drv, TMNTCore &core) : driver(drv), main_core(core)
    {

    }

    TMNTM68K::~TMNTM68K()
    {

    }

    void TMNTM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void TMNTM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t TMNTM68K::readCPU16(bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;
	if (addr < 0x60000)
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
	else if (inRange(addr, 0x60000, 0x64000))
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
	else if (inRange(addr, 0x80000, 0x81000))
	{
	    if (lower)
	    {
		data = main_core.readPalette((addr >> 1));
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
	else if (addr == 0xA0006)
	{
	    // P3
	    if (lower)
	    {
		data = main_core.readDIP(3);
	    }
	}
	else if (addr == 0xA0010)
	{
	    // DSW1
	    if (lower)
	    {
		data = main_core.readDIP(5);
	    }
	}
	else if (addr == 0xA0012)
	{
	    // DSW2
	    if (lower)
	    {
		data = main_core.readDIP(6);
	    }
	}
	else if (addr == 0xA0014)
	{
	    // P4
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
		data = main_core.readDIP(7);
	    }
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    data = main_core.readk052109_noA12(upper, lower, (addr >> 1));
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

    void TMNTM68K::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x60000)
	{
	    return;
	}
	else if (inRange(addr, 0x60000, 0x64000))
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
	else if (inRange(addr, 0x80000, 0x81000))
	{
	    if (lower)
	    {
		main_core.writePalette((addr >> 1), data);
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
	    main_core.writePriority(data);
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    main_core.writek052109_noA12(upper, lower, (addr >> 1), data);
	}
	else if (addr == 0x10E800)
	{
	    return; // ???
	}
	else if (inRange(addr, 0x140000, 0x1407FF))
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

    TMNTZ80::TMNTZ80(berrndriver &drv, TMNTCore &core) : driver(drv), sound_core(core)
    {
	opm = new ym2151device(driver);
	k007232 = new k007232device(driver);
	title = new samplesdevice(driver);
    }

    TMNTZ80::~TMNTZ80()
    {

    }

    void TMNTZ80::init()
    {
	opm->init(3579545);
	k007232->init(3579545);

	k007232->setVolumeCallback([&](uint8_t data) -> void
	{
	    k007232->setVolume((data >> 4), (data & 0xF));
	});

	title->init();
	initTitle();
	sound_rom = driver.get_rom_region("soundcpu");
	sound_ram.fill(0);
    }

    void TMNTZ80::shutdown()
    {
	title->stop_sound(title_id);
	title->shutdown();
	k007232->shutdown();
	opm->shutdown();
	sound_rom.clear();
    }

    void TMNTZ80::initTitle()
    {
	auto sample_data = driver.get_rom_region("title");
	for (int i = 0; i < 0x80000; i += 2)
	{
	    int val = ((sample_data.at(i + 1) << 8) | sample_data.at(i));
	    title_samples.push_back(beenukedutil::decode_fp(val >> 3));
	}

	title_id = title->load_raw(title_samples, 0x40000, 20000);
    }

    uint8_t TMNTZ80::readCPU8(uint16_t addr)
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
	else if (addr == 0x9000)
	{
	    data = internal_latch;
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
	    // cout << "Reading YM2151 port of " << dec << int((addr - 0xC000)) << endl;
	    int port = (addr - 0xC000);
	    data = opm->readIO(port);
	}
	else if (addr == 0xF000)
	{
	    cout << "Reading uPD7759 busy line..." << endl;
	    data = 0x00;
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void TMNTZ80::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x8000)
	{
	    return;
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    sound_ram.at(addr & 0x7FF) = data;
	}
	else if (addr == 0x9000)
	{
	    if (testbit(data, 1))
	    {
		cout << "Asserting uPD7759 reset line..." << endl;
	    }
	    else
	    {
		cout << "Clearing uPD7759 reset line..." << endl;
	    }

	    if (testbit(data, 2))
	    {
		if (!title->is_playing(title_id))
		{
		    title->play_sound(title_id);
		}
	    }
	    else
	    {
		title->stop_sound(title_id);
	    }

	    internal_latch = data;
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
	else if (addr == 0xD000)
	{
	    cout << "Writing value of " << hex << int(data) << " to uPD7759 port" << endl;
	}
	else if (addr == 0xE000)
	{
	    if (testbit(data, 0))
	    {
		cout << "Asserting uPD7759 start line..." << endl;
	    }
	    else
	    {
		cout << "Clearing uPD7759 start line..." << endl;
	    }
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    void TMNTZ80::processAudio()
    {
	auto opm_samples = opm->fetch_samples();
	auto k007232_samples = k007232->fetch_samples();
	auto title_samples = title->fetch_samples();

	for (auto &ym_sample : opm_samples)
	{
	    driver.add_mono_sample(ym_sample);
	}

	for (auto &k007232_sample : k007232_samples)
	{
	    driver.add_mono_sample(k007232_sample, 0.33);
	}

	for (auto &title_sample : title_samples)
	{
	    driver.add_mono_sample(title_sample, 0.5);
	}
    }

    TMNTCore::TMNTCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();

	main_inter = new TMNTM68K(driver, *this);
	main_proc = new BerrnM68KProcessor(8000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	sound_inter = new TMNTZ80(driver, *this);
	sound_proc = new BerrnZ80Processor(3579545, *sound_inter);
	sound_cpu = new BerrnCPU(scheduler, *sound_proc);

	video = new tmntvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    if (is_irq_enabled)
	    {
		main_proc->fire_interrupt_level(5);
	    }

	    video->updatePixels();

	    vblank_start_time = driver.get_scheduler().get_current_time();
	    vblank_timer->start(time_until_pos(240), false);
	});
    }

    TMNTCore::~TMNTCore()
    {

    }

    int64_t TMNTCore::time_until_pos(int vpos)
    {
	auto &scheduler = driver.get_scheduler();
	vpos = ((vpos + 16) % 256);

	int64_t scan_time = (time_in_hz(60) / 256);

	int64_t target_delta = (vpos * scan_time);

	int64_t current_delta = (scheduler.get_current_time() - vblank_start_time);

	while (target_delta <= current_delta)
	{
	    target_delta += time_in_hz(60);
	}

	return (target_delta - current_delta);
    }

    bool TMNTCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_inter->init();
	main_proc->init();
	sound_inter->init();
	sound_proc->init();
	scheduler.add_device(main_cpu);
	scheduler.add_device(sound_cpu);
	vblank_timer->start(time_until_pos(240), false);
	video->init();
	coins_port = 0xFF;
	p1_port = 0xFF;
	driver.resize(320, 224, 2);
	return true;
    }

    void TMNTCore::stop_core()
    {
	video->shutdown();
	vblank_timer->stop();
	sound_inter->shutdown();
	sound_proc->shutdown();
	main_inter->shutdown();
	main_proc->shutdown();
    }

    void TMNTCore::run_core()
    {
	driver.run_scheduler();
    }

    void TMNTCore::key_changed(BerrnInput key, bool is_pressed)
    {
	string key_state = (is_pressed) ? "pressed" : "released";

	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		coins_port = changebit(coins_port, 0, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1: break;
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
	    default: break;
	}
    }

    void TMNTCore::process_audio()
    {
	sound_inter->processAudio();
    }

    uint8_t TMNTCore::readPalette(uint32_t addr)
    {
	return video->palette_read(addr);
    }

    void TMNTCore::writePalette(uint32_t addr, uint8_t data)
    {
	video->palette_write(addr, data);
    }

    void TMNTCore::write0A0000(uint16_t data)
    {
	if (prev_int && !testbit(data, 3))
	{
	    sound_proc->fire_interrupt8();
	}

	prev_int = testbit(data, 3);

	is_irq_enabled = testbit(data, 5);
	video->setRMRD(testbit(data, 7));
    }

    uint16_t TMNTCore::readk052109_noA12(bool upper, bool lower, uint32_t addr)
    {
	addr = ((addr & 0x7FF) | ((addr & 0x3000) >> 1) | (lower << 13) | (addr & 0xC000));
	uint16_t read_data = video->tile_read(addr);

	if (upper)
	{
	    read_data <<= 8;
	}

	if (lower)
	{
	    read_data &= 0xFF;
	}

	return read_data;
    }

    void TMNTCore::writek052109_noA12(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	addr = ((addr & 0x7FF) | ((addr & 0x3000) >> 1) | (lower << 13) | (addr & 0xC000));
	uint8_t write_data = 0;

	if (upper)
	{
	    write_data = (data >> 8);
	}

	if (lower)
	{
	    write_data = (data & 0xFF);
	}

	video->tile_write(addr, write_data);
    }

    uint8_t TMNTCore::k051960_read(uint16_t addr)
    {
	return video->sprite_read(addr);
    }

    void TMNTCore::k051960_write(uint16_t addr, uint8_t data)
    {
	video->sprite_write(addr, data);
    }

    void TMNTCore::writePriority(uint8_t data)
    {
	int priority_flag = ((data >> 2) & 0x3);
	video->set_priority(priority_flag);
    }

    uint8_t TMNTCore::readDIP(int bank)
    {
	uint8_t data = 0;
	switch (bank)
	{
	    case 0: data = coins_port; break; // COINS
	    case 1: data = p1_port; break; // P1
	    case 2: data = 0xFF; break; // P2
	    case 3: data = 0xFF; break; // P3
	    case 4: data = 0xFF; break; // P4
	    case 5: data = 0xFF; break; // DSW1
	    case 6: data = 0x5E; break; // DSW2
	    case 7: data = 0xFF; break; // DSW3
	    default: break; 
	}

	return data;
    }

    uint8_t TMNTCore::readSoundLatch()
    {
	return sound_cmd;
    }

    void TMNTCore::writeSoundLatch(uint8_t data)
    {
	sound_cmd = data;
    }

    drivertmnt::drivertmnt()
    {
	core = new TMNTCore(*this);
    }

    drivertmnt::~drivertmnt()
    {

    }

    string drivertmnt::drivername()
    {
	return "tmnt";
    }

    bool drivertmnt::drvinit()
    {
	if (!loadROM(berrn_rom_name(tmnt)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivertmnt::drvshutdown()
    {
	core->stop_core();
    }
  
    void drivertmnt::drvrun()
    {
	core->run_core();
    }

    void drivertmnt::process_audio()
    {
	core->process_audio();
    }

    void drivertmnt::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }
};