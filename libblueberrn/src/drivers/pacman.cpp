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

#include "pacman.h"
using namespace berrn;

namespace berrn
{
    PacmanInterface::PacmanInterface()
    {

    }

    PacmanInterface::~PacmanInterface()
    {

    }

    void PacmanInterface::init()
    {
	gamerom.resize(0x4000, 0);
	vram.fill(0);
	cram.fill(0);
	mainram.fill(0);
    }

    void PacmanInterface::shutdown()
    {
	gamerom.clear();
    }

    uint8_t PacmanInterface::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void PacmanInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FFF;

	if (addr < 0x4000)
	{
	    return;
	}
	else if (addr < 0x4400)
	{
	    vram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x4800)
	{
	    cram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x4C00)
	{
	    return;
	}
	else if (addr < 0x4FF0)
	{
	    mainram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x5000)
	{
	    oam[(addr & 0xF)] = data;
	}
	else if (addr < 0x5040)
	{
	    writeIO((addr & 7), data);
	}
	else if (addr < 0x5060)
	{
	    // TODO: Implement sound support
	    return;
	}
	else if (addr < 0x5070)
	{
	    sprite_pos[(addr & 0xF)] = data;
	}
	else if (addr < 0x50C0)
	{
	    return;
	}
	else
	{
	    // Watchdog timer (currently unimplemented)
	    return;
	}
    }

    uint8_t PacmanInterface::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t PacmanInterface::readByte(uint16_t addr)
    {
	addr &= 0x7FFF;

	uint8_t data = 0;

	if (addr < 0x4000)
	{
	    data = gamerom[addr];
	}
	else if (addr < 0x4400)
	{
	    data = vram[(addr & 0x3FF)];
	}
	else if (addr < 0x4800)
	{
	    data = cram[(addr & 0x3FF)];
	}
	else if (addr < 0x4C00)
	{
	    data = 0x00;
	}
	else if (addr < 0x4FF0)
	{
	    data = mainram[(addr & 0x3FF)];
	}
	else if (addr < 0x5000)
	{
	    data = oam[(addr & 0xF)];
	}
	else if (addr < 0x5040)
	{
	    cout << "Reading IN0" << endl;
	    data = 0xFF;
	}
	else if (addr < 0x5080)
	{
	    cout << "Reading IN1" << endl;
	    data = 0xFF;
	}
	else if (addr < 0x50C0)
	{
	    // Reading IN2
	    data = 0xC9;
	}
	else
	{
	    cout << "Reading from address of " << hex << int(addr) << endl;
	    exit(0);
	}

	return data;
    }

    void PacmanInterface::writeIO(int addr, uint8_t data)
    {
	switch (addr)
	{
	    case 0: vblank_enable = testbit(data, 0); break;
	    case 1: sound_enable = testbit(data, 0); break;
	    case 3: flip_screen = testbit(data, 0); break;
	}
    }

    void PacmanInterface::portOut(uint16_t port, uint8_t data)
    {
	if ((port & 0xFF) == 0)
	{
	    int_vector = data;
	}
	else
	{
	    BerrnInterface::portOut(port, data);
	}
    }

    void PacmanInterface::updatePixels()
    {
	return;
    }

    driverpacman::driverpacman()
    {
	pacman_proc = new BerrnZ80Processor(3072000, inter);
	pacman_cpu = new BerrnCPU(scheduler, *pacman_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t) {
	    inter.updatePixels();
	});

	interrupt_timer = new BerrnTimer("Interrupt", scheduler, [&](int64_t param, int64_t) {
	    if (param == 1)
	    {
		interrupt_timer->start(16500, true);
	    }

	    this->interrupt_handler();
	});
    }

    driverpacman::~driverpacman()
    {

    }

    string driverpacman::drivername()
    {
	return "pacman";
    }

    bool driverpacman::hasdriverROMs()
    {
	return true;
    }

    void driverpacman::loadROMs()
    {
	loadROM("pacman.6e", 0x0000, 0x1000, inter.get_gamerom());
	loadROM("pacman.6f", 0x1000, 0x1000, inter.get_gamerom());
	loadROM("pacman.6h", 0x2000, 0x1000, inter.get_gamerom());
	loadROM("pacman.6j", 0x3000, 0x1000, inter.get_gamerom());
    }

    void driverpacman::interrupt_handler()
    {
	if (inter.is_vblank_enabled())
	{
	    pacman_proc->fire_interrupt8(inter.get_int_vec());
	}
    }

    bool driverpacman::drvinit()
    {
	loadROMs();
	scheduler.reset();
	scheduler.add_device(pacman_cpu);
	interrupt_timer->start(14000, false, 1);
	vblank_timer->start(16500, true);
	pacman_proc->init();
	inter.init();
	resize(224, 288, 2);
	return isallfilesloaded();
    }

    void driverpacman::drvshutdown()
    {
	vblank_timer->stop();
	inter.shutdown();
	pacman_proc->shutdown();
	scheduler.remove_timer(vblank_timer);
	scheduler.remove_timer(interrupt_timer);
	scheduler.remove_device(pacman_cpu);
    }
  
    void driverpacman::drvrun()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = 16500;

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}

	fillrect(0, 0, 224, 288, black());
    }

    void driverpacman::keychanged(BerrnInput key, bool is_pressed)
    {
	return;
    }
};