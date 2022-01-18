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

#include <invaders.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    berrn_rom_start(invaders)
	berrn_rom_region("cpu", 0x2000)
	    berrn_rom_load("invaders.h", 0x0000, 0x0800)
	    berrn_rom_load("invaders.g", 0x0800, 0x0800)
	    berrn_rom_load("invaders.f", 0x1000, 0x0800)
	    berrn_rom_load("invaders.e", 0x1800, 0x0800)
    berrn_rom_end

    InvadersCore::InvadersCore(berrndriver &drv) : driver(drv)
    {
	main_proc = new Berrn8080Processor(2000000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t) {
	    update_pixels();
	});

	interrupt_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t) {
	    uint8_t interrupt_op = (is_end_of_frame) ? 0xD7 : 0xCF;
	    main_proc->fire_interrupt(interrupt_op);
	    is_end_of_frame = !is_end_of_frame;
	});

	sound_timer = new BerrnTimer("Sound", scheduler, [&](int64_t, int64_t)
	{
	    driver.mixSample(driver.getRawSample());
	    driver.outputAudio();
	});

	port1_val = 0x01;

	bitmap = new BerrnBitmapRGB(224, 256);
	bitmap->clear();
    }

    InvadersCore::~InvadersCore()
    {

    }

    bool InvadersCore::init_core()
    {
	main_rom = driver.get_rom_region("cpu");
	scheduler.reset();
	scheduler.add_device(main_cpu);
	main_proc->init();

	load_sound("0.wav"); // UFO (repeats)
	load_sound("1.wav"); // Shot
	load_sound("2.wav"); // Player explosion
	load_sound("3.wav"); // Invaders explosion
	load_sound("4.wav"); // Fleet movement 1
	load_sound("5.wav"); // Fleet movement 2
	load_sound("6.wav"); // Fleet movement 3
	load_sound("7.wav"); // Fleet movement 4
	load_sound("8.wav"); // UFO hit
	load_sound("9.wav"); // Extended play

	driver.setSoundLoop(sound_IDs[0], true);

	vblank_timer->start(time_in_hz(60), true);
	interrupt_timer->start(time_in_hz(120), true);
	sound_timer->start(time_in_hz(driver.getSampleRate()), true);

	driver.resize(224, 256, 2);
	return true;
    }

    void InvadersCore::shutdown_core()
    {
	work_ram.fill(0);
	video_ram.fill(0);
	main_proc->shutdown();
	sound_timer->stop();
	vblank_timer->stop();
	interrupt_timer->stop();
	scheduler.shutdown();
	bitmap->clear();
    }

    void InvadersCore::load_sound(string filename)
    {
	int sound_id = driver.loadWAV(filename);
	driver.setSoundVol(sound_id, 0.25);
	sound_IDs.push_back(sound_id);
    }

    void InvadersCore::run_core()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = time_in_hz(60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}
    }

    void InvadersCore::update_pixels()
    {
	for (uint32_t addr = 0; addr < 0x1C00; addr++)
	{
	    int ypos = ((addr * 8) / 256);
	    int base_x = ((addr * 8) % 256);

	    uint8_t video_byte = video_ram.at(addr);

	    for (int bit = 0; bit < 8; bit++)
	    {
		int xpos = (255 - (base_x + bit));
		berrnRGBA color = testbit(video_byte, bit) ? white() : black();
		bitmap->setPixel(ypos, xpos, color);
	    }
	}

	driver.setScreen(bitmap);
    }

    uint8_t InvadersCore::readCPU8(uint16_t addr)
    {
	addr &= 0x3FFF;
	uint8_t data = 0;
	if (addr < 0x2000)
	{
	    data = main_rom.at(addr);
	}
	else if (addr < 0x2400)
	{
	    data = work_ram.at((addr - 0x2000));
	}
	else
	{
	    data = video_ram.at((addr - 0x2400));
	}
	
	return data;
    }

    void InvadersCore::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FFF;

	if (addr < 0x2000)
	{
	    return;
	}
	else if (addr < 0x2400)
	{
	    work_ram.at((addr - 0x2000)) = data;
	}
	else
	{
	    video_ram.at((addr - 0x2400)) = data;
	}
    }

    uint8_t InvadersCore::portIn(uint16_t port)
    {
	uint8_t data = 0;
	port &= 3;

	switch (port)
	{
	    case 1: data = port1_val; break;
	    case 2: data = 0x00; break;
	    case 3: data = shifter.readshiftresult(); break;
	    default: data = BerrnInterface::portIn(port); break;
	}

	return data;
    }

    void InvadersCore::portOut(uint16_t port, uint8_t val)
    {
	port &= 7;

	switch (port)
	{
	    case 2: shifter.setshiftoffs(val); break;
	    case 3: write_sound_port(0, val); break;
	    case 4: shifter.fillshiftreg(val); break;
	    case 5: write_sound_port(1, val); break;
	    case 6: /* debugPort(val); */ break;
	    default: BerrnInterface::portOut(port, val); break;
	}
    }

    bool InvadersCore::is_rising_edge(uint8_t data, uint8_t prev_data, int bit)
    {
	return (testbit(data, bit) && !testbit(prev_data, bit));
    }

    bool InvadersCore::is_falling_edge(uint8_t data, uint8_t prev_data, int bit)
    {
	return (!testbit(data, bit) && testbit(prev_data, bit));
    }

    void InvadersCore::write_sound_port(int bank, uint8_t data)
    {
	if (bank == 0)
	{
	    if (is_rising_edge(data, prev_port3, 0))
	    {
		play_sound(0, true);
	    }
	    else if (is_falling_edge(data, prev_port3, 0))
	    {
		play_sound(0, false);
	    }

	    if (is_rising_edge(data, prev_port3, 1))
	    {
		play_sound(1);
	    }

	    if (is_rising_edge(data, prev_port3, 2))
	    {
		play_sound(2);
	    }

	    if (is_rising_edge(data, prev_port3, 3))
	    {
		play_sound(3);
	    }

	    if (is_rising_edge(data, prev_port3, 4))
	    {
		play_sound(9);
	    }

	    prev_port3 = data;
	}
	else if (bank == 1)
	{
	    if (is_rising_edge(data, prev_port5, 0))
	    {
		play_sound(4);
	    }

	    if (is_rising_edge(data, prev_port5, 1))
	    {
		play_sound(5);
	    }

	    if (is_rising_edge(data, prev_port5, 2))
	    {
		play_sound(6);
	    }

	    if (is_rising_edge(data, prev_port5, 3))
	    {
		play_sound(7);
	    }

	    if (is_rising_edge(data, prev_port5, 4))
	    {
		play_sound(8);
	    }

	    prev_port5 = data;
	}
    }

    void InvadersCore::play_sound(int id, bool is_playing)
    {
	auto sound_id = sound_IDs[id];

	if (sound_id == -1)
	{
	    return;
	}

	if (is_playing)
	{
	    driver.playSound(sound_id);
	}
	else
	{
	    driver.stopSound(sound_id);
	}
    }

    void InvadersCore::debugPort(uint8_t val)
    {
	char debug_char = (val <= 25) ? ('A' + val) : '\n';
	cout.put(debug_char);
	fflush(stdout);
    }

    void InvadersCore::key_changed(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		port1_val = changebit(port1_val, 0, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		port1_val = changebit(port1_val, 2, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		port1_val = changebit(port1_val, 5, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		port1_val = changebit(port1_val, 6, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnFireP1:
	    {
		port1_val = changebit(port1_val, 4, is_pressed);
	    }
	    break;
	    default: break;
	}
    }

    driverinvaders::driverinvaders()
    {
	inv_core = new InvadersCore(*this);
    }

    driverinvaders::~driverinvaders()
    {

    }

    string driverinvaders::drivername()
    {
	return "invaders";
    }

    bool driverinvaders::hasdriverROMs()
    {
	return true;
    }

    bool driverinvaders::drvinit()
    {
	if (!loadROM(berrn_rom_name(invaders)))
	{
	    return false;
	}

	return inv_core->init_core();
    }

    void driverinvaders::drvshutdown()
    {
	inv_core->shutdown_core();
    }

    void driverinvaders::drvrun()
    {
	inv_core->run_core();
    }

    void driverinvaders::keychanged(BerrnInput key, bool is_pressed)
    {
	inv_core->key_changed(key, is_pressed);
    }
};