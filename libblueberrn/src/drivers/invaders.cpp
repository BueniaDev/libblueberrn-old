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

#include <invaders.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    InvadersInterface::InvadersInterface()
    {

    }

    InvadersInterface::~InvadersInterface()
    {

    }

    void InvadersInterface::init()
    {
	framebuffer.fill(black());
	workram.resize(0x400, 0);
	videoram.resize(0x1C00, 0);
	port1_val = 8;
    }

    void InvadersInterface::shutdown()
    {
	workram.clear();
	videoram.clear();
    }

    uint8_t InvadersInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x3FFF;

	if (addr < 0x2000)
	{
	    data = gamerom.at(addr);
	}
	else if (addr < 0x2400)
	{
	    data = workram.at((addr & 0x3FF));
	}
	else
	{
	    data = videoram.at((addr - 0x2400));
	}

	return data;
    }

    void InvadersInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FFF;

	if (addr < 0x2000)
	{
	    return;
	}
	else if (addr < 0x2400)
	{
	    workram.at((addr & 0x3FF)) = data;
	}
	else
	{
	    videoram.at((addr - 0x2400)) = data;
	}
    }

    uint8_t InvadersInterface::portIn(uint16_t port)
    {
	// TODO: Implement rest of user input
	uint8_t data = 0;

	switch (port)
	{
	    case 1: data = port1_val; break;
	    case 2: data = 0; break;
	    case 3: data = shifter.readshiftresult(); break;
	    default: data = BerrnInterface::portIn(port); break;
	}

	return data;
    }

    void InvadersInterface::portOut(uint16_t port, uint8_t val)
    {
	switch (port)
	{
	    case 2: shifter.setshiftoffs(val); break;
	    case 3: break;
	    case 4: shifter.fillshiftreg(val); break;
	    case 5: break;
	    case 6: /* debugPort(val); */ break;
	    default: BerrnInterface::portOut(port, val); break;
	}
    }

    void InvadersInterface::updatePixels()
    {
	for (int i = 0; i < ((256 * 224) / 8); i++)
	{
	    int y = ((i * 8) / 256);
	    int basex = ((i * 8) % 256);

	    uint8_t current_byte = videoram.at(i);

	    for (int bit = 0; bit < 8; bit++)
	    {
		int pixelx = y;
		int pixely = (255 - (basex + bit));

		int index = (pixelx + (pixely * width));
		berrnRGBA color = testbit(current_byte, bit) ? white() : black();
		framebuffer.at(index) = color;
	    }
	}
    }

    void InvadersInterface::debugPort(uint8_t val)
    {
	char debugchar = (val <= 25) ? ('A' + val) : '\n';
	cout.put(debugchar);
	fflush(stdout);
    }

    driverinvaders::driverinvaders()
    {
	invaders_proc = new Berrn8080Processor(2000000, inter);
	invaders_cpu = new BerrnCPU(scheduler, *invaders_proc);

	auto int_func = [&](int64_t, int64_t) {
	    this->interrupt_handler();
        };

	interrupt_timer = new BerrnTimer("Interrupt", scheduler, int_func);
    }

    driverinvaders::~driverinvaders()
    {

    }

    void driverinvaders::interrupt_handler()
    {
	uint8_t interrupt_op = (is_end_of_frame) ? 0xD7 : 0xCF;
	invaders_proc->fire_interrupt(interrupt_op);
	is_end_of_frame = !is_end_of_frame;
    }

    string driverinvaders::drivername()
    {
	return "invaders";
    }

    bool driverinvaders::hasdriverROMs()
    {
	return true;
    }

    void driverinvaders::loadROMs()
    {
	loadROM("invaders.h", 0x0000, 0x0800, inter.get_gamerom());
	loadROM("invaders.g", 0x0800, 0x0800, inter.get_gamerom());
	loadROM("invaders.f", 0x1000, 0x0800, inter.get_gamerom());
	loadROM("invaders.e", 0x1800, 0x0800, inter.get_gamerom());
    }

    bool driverinvaders::drvinit()
    {
	loadROMs();
	scheduler.reset();
	scheduler.add_device(invaders_cpu);
	interrupt_timer->start(((1e6 / 60) / 2), true);
	resize(224, 256, 2);
	invaders_proc->init();
	inter.init();
	is_end_of_frame = false;
    
	return isallfilesloaded();
    }

    void driverinvaders::drvshutdown()
    {
	interrupt_timer->stop();
	inter.shutdown();
	invaders_proc->shutdown();
	scheduler.remove_timer(interrupt_timer);
	scheduler.remove_device(invaders_cpu);
    }
  
    void driverinvaders::drvrun()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = (1e6 / 60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}

	inter.updatePixels();
	filltexrect(0, 0, 224, 256, inter.get_framebuffer());
    }

    void driverinvaders::keychanged(BerrnInput key, bool is_pressed)
    {
	return;
    }
};