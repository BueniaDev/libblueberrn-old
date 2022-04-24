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
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("invaders.h", 0x0000, 0x0800)
	    berrn_rom_load("invaders.g", 0x0800, 0x0800)
	    berrn_rom_load("invaders.f", 0x1000, 0x0800)
	    berrn_rom_load("invaders.e", 0x1800, 0x0800)
    berrn_rom_end

    InvadersCore::InvadersCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();

	main_proc = new Berrn8080Processor(2000000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    updatePixels();
	});

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    uint8_t interrupt_op = (is_end_of_frame) ? 0xD7 : 0xCF;
	    main_proc->fire_interrupt8(interrupt_op);
	    is_end_of_frame = !is_end_of_frame;
	});

	bitmap = new BerrnBitmapRGB(256, 224);
	bitmap->clear();
    }

    InvadersCore::~InvadersCore()
    {

    }

    bool InvadersCore::initcore()
    {
	port1 = 0x00;
	main_proc->init();
	rom = driver.get_rom_region("maincpu");
	auto &scheduler = driver.get_scheduler();

	main_ram.fill(0);
	video_ram.fill(0);

	scheduler.add_device(main_cpu);
	vblank_timer->start(time_in_hz(60), true);
	irq_timer->start(time_in_hz(120), true);
	driver.resize(256, 224, 2);
	return true;
    }

    void InvadersCore::stopcore()
    {
	rom.clear();
	vblank_timer->stop();
	irq_timer->stop();
	main_proc->shutdown();
    }

    void InvadersCore::runcore()
    {
	driver.run_scheduler();
    }

    void InvadersCore::keychanged(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		port1 = changebit(port1, 0, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		port1 = changebit(port1, 2, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		port1 = changebit(port1, 5, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		port1 = changebit(port1, 6, is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnFireP1:
	    {
		port1 = changebit(port1, 4, is_pressed);
	    }
	    break;
	    default: break;
	}
    }

    uint8_t InvadersCore::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x3FFF;

	if (addr < 0x2000)
	{
	    data = rom.at(addr);
	}
	else if (addr < 0x2400)
	{
	    data = main_ram.at(addr - 0x2000);
	}
	else
	{
	    data = video_ram.at(addr - 0x2400);
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
	    main_ram.at(addr - 0x2000) = data;
	}
	else
	{
	    video_ram.at(addr - 0x2400) = data;
	}
    }

    uint8_t InvadersCore::portIn(uint16_t port)
    {
	uint8_t data = 0;
	port &= 3;

	switch (port)
	{
	    case 0: data = 0x08; break;
	    case 1: data = port1; break;
	    case 2: data = 0x00; break;
	    case 3: data = shifter.readshiftresult(); break;
	}

	return data;
    }

    void InvadersCore::portOut(uint16_t port, uint8_t data)
    {
	port &= 7;

	switch (port)
	{
	    case 2: shifter.setshiftoffs(data); break;
	    case 3: break;
	    case 4: shifter.fillshiftreg(data); break;
	    case 5: break;
	    case 6: /* debugPort(data); */ break;
	    default: BerrnInterface::portOut(port, data); break;
	}
    }

    void InvadersCore::debugPort(uint8_t data)
    {
	char debug_char = (data <= 25) ? ('A' + data) : '\n';
	cout.put(debug_char);
	fflush(stdout);
    }

    void InvadersCore::updatePixels()
    {
	for (int i = 0; i < 0x1C00; i++)
	{
	    int ypos = ((i * 8) / 256);
	    int base_x = ((i * 8) % 256);

	    uint8_t vram_byte = video_ram.at(i);

	    for (int bit = 0; bit < 8; bit++)
	    {
		int xpos = (base_x + bit);
		berrnRGBA color = testbit(vram_byte, bit) ? white() : black();
		bitmap->setPixel(xpos, ypos, color);
	    }
	}

	driver.set_screen(bitmap);
    }

    driverinvaders::driverinvaders()
    {
	core = new InvadersCore(*this);
    }

    driverinvaders::~driverinvaders()
    {

    }

    string driverinvaders::drivername()
    {
	return "invaders";
    }

    uint32_t driverinvaders::get_flags()
    {
	return berrn_rot_270;
    }

    bool driverinvaders::drvinit()
    {
	if (!loadROM(berrn_rom_name(invaders)))
	{
	    return false;
	}

	return core->initcore();
    }

    void driverinvaders::drvshutdown()
    {
	core->stopcore();
    }
  
    void driverinvaders::drvrun()
    {
	core->runcore();
    }

    void driverinvaders::keychanged(BerrnInput key, bool is_pressed)
    {
	core->keychanged(key, is_pressed);
    }
};