/*
    This file is part of libblueberrn.
    Copyright (C) 2021 BueniaDev.

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

#include "galaxian.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(galaxian)
	berrn_rom_region("cpu", 0x4000)
	    berrn_rom_load("galmidw.u", 0x0000, 0x0800)
	    berrn_rom_load("galmidw.v", 0x0800, 0x0800)
	    berrn_rom_load("galmidw.w", 0x1000, 0x0800)
	    berrn_rom_load("galmidw.y", 0x1800, 0x0800)
	    berrn_rom_load("7l", 0x2000, 0x0800)
	berrn_rom_region("gfx", 0x1000)
	    berrn_rom_load("1h.bin", 0x0000, 0x0800)
	    berrn_rom_load("1k.bin", 0x0800, 0x0800)
	berrn_rom_region("pal", 0x0020)
	    berrn_rom_load("6l.bpr", 0x0000, 0x0020)
    berrn_rom_end

    GalaxianInterface::GalaxianInterface(berrndriver &drv) : driver(drv)
    {
	main_proc = new BerrnZ80Processor(3072000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	video_core = new galaxianvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t) {
	    update_pixels();
	});

	interrupt_timer = new BerrnTimer("Interrupt", scheduler, [&](int64_t param, int64_t)
	{
	    if (param == 1)
	    {
		interrupt_timer->start(16500, true);
	    }

	    if (is_int_enabled)
	    {
		main_proc->fire_nmi();
	    }
	});
    }

    GalaxianInterface::~GalaxianInterface()
    {

    }

    bool GalaxianInterface::init_core()
    {
	scheduler.reset();
	scheduler.add_device(main_cpu);
	main_proc->init();

	video_core->init();

	vblank_timer->start(16500, true);
	interrupt_timer->start(14000, false, 1);

	driver.resize(224, 256, 2);
	ram.fill(0);

	// Reset IO enable registers
	for (int i = 0; i < 8; i++)
	{
	    write_upperIO(i, 0);
	}

	game_rom = driver.get_rom_region("cpu");

	return true;
    }

    void GalaxianInterface::shutdown_core()
    {
	main_proc->shutdown();
	video_core->shutdown();
	interrupt_timer->stop();
	vblank_timer->stop();
	ram.fill(0);
	scheduler.remove_timer(interrupt_timer);
	scheduler.remove_timer(vblank_timer);
	scheduler.remove_device(main_cpu);
    }

    void GalaxianInterface::run_core()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = 16500;

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}
    }

    void GalaxianInterface::key_changed(BerrnInput key, bool is_pressed)
    {

    }

    void GalaxianInterface::update_pixels()
    {
	video_core->update_pixels();
	driver.setScreen(video_core->get_bitmap());
    }

    uint8_t GalaxianInterface::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void GalaxianInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	writeByte(addr, data);
    }

    uint8_t GalaxianInterface::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t GalaxianInterface::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x4000)
	{
	    data = game_rom.at(addr);
	}
	else if (inRange(addr, 0x4000, 0x5000))
	{
	    data = ram[(addr & 0x3FF)];
	}
	else if (inRange(addr, 0x5000, 0x6000))
	{
	    data = video_core->readByte(addr);
	}
	else if (inRange(addr, 0x6000, 0x6800))
	{
	    // TODO: IN0 reads
	    data = 0x00;
	}
	else if (inRange(addr, 0x6800, 0x7000))
	{
	    // TODO: IN1 reads
	    data = 0x00;
	}
	else if (inRange(addr, 0x7000, 0x7800))
	{
	    // IN2 port values
	    //
	    // Bits 0-1: Bonus life at:
	    // 0 = 7000 points
	    // 1 = 10000 points
	    // 2 = 12000 points
	    // 3 = 20000 points
	    //
	    // Bit 2 - Number of lives per individual game
	    // 0 = 2 lives
	    // 1 = 3 lives

	    data = 0x04;
	}
	else if (inRange(addr, 0x7800, 0x8000))
	{
	    // Watchdog timer (unimplemented)
	    data = 0xFF;
	}
	else
	{
	    data = 0xFF;
	}

	return data;
    }

    void GalaxianInterface::writeByte(uint16_t addr, uint8_t data)
    {
	if (addr < 0x4000)
	{
	    return;
	}
	else if (inRange(addr, 0x4000, 0x5000))
	{
	    ram[(addr & 0x3FF)] = data;
	}
	else if (inRange(addr, 0x5000, 0x6000))
	{
	    video_core->writeByte(addr, data);
	}
	else if (inRange(addr, 0x6000, 0x6800))
	{
	    write_lowerIO((addr & 7), data);
	}
	else if (inRange(addr, 0x6800, 0x7000))
	{
	    cout << "Writing value of " << hex << int(data) << " to possible sound-write register of " << dec << int((addr & 0x7)) << endl;
	}
	else if (inRange(addr, 0x7000, 0x7800))
	{
	    write_upperIO((addr & 7), data);
	}
	else if (inRange(addr, 0x7800, 0x8000))
	{
	    cout << "Writing value of " << hex << int(data) << " to possible pitch-write register" << endl;
	}
	else
	{
	    return;
	}
    }

    void GalaxianInterface::write_lowerIO(int addr, uint8_t data)
    {
	switch (addr)
	{
	    case 0:
	    case 1:
	    case 2:
	    case 3: return; break;
	    default:
	    {
		cout << "Writing value of " << hex << int(data) << " to possible lfo-freq register of " << dec << int(addr) << endl; break;
	    }
	    break;
	}
    }

    void GalaxianInterface::write_upperIO(int addr, uint8_t data)
    {
	switch (addr)
	{
	    case 1: is_int_enabled = testbit(data, 0); break;
	    case 4: video_core->update_latch(0, data); break;
	    case 6: video_core->update_latch(1, data); break;
	    case 7: video_core->update_latch(2, data); break;
	}
    }

    drivergalaxian::drivergalaxian()
    {
	inter = new GalaxianInterface(*this);
    }

    drivergalaxian::~drivergalaxian()
    {

    }

    string drivergalaxian::drivername()
    {
	return "galaxian";
    }

    bool drivergalaxian::hasdriverROMs()
    {
	return true;
    }

    bool drivergalaxian::drvinit()
    {
	if (!loadROM(berrn_rom_name(galaxian)))
	{
	    return false;
	}

	return inter->init_core();
    }

    void drivergalaxian::drvshutdown()
    {
	inter->shutdown_core();
    }
  
    void drivergalaxian::drvrun()
    {
	inter->run_core();
    }

    float drivergalaxian::get_framerate()
    {
	return (16000.0 / 132.0 / 2.0); // Framerate is 60.606060 Hz
    }

    void drivergalaxian::keychanged(BerrnInput key, bool is_pressed)
    {
	inter->key_changed(key, is_pressed);
    }
};