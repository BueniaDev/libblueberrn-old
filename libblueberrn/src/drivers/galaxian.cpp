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

#include "galaxian.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(galaxian)
	berrn_rom_region("maincpu", 0x4000, 0)
	    berrn_rom_load("galmidw.u", 0x0000, 0x0800)
	    berrn_rom_load("galmidw.v", 0x0800, 0x0800)
	    berrn_rom_load("galmidw.w", 0x1000, 0x0800)
	    berrn_rom_load("galmidw.y", 0x1800, 0x0800)
	    berrn_rom_load("7l",        0x2000, 0x0800)
	berrn_rom_region("gfx", 0x1000, 0)
	    berrn_rom_load("1h.bin",    0x0000, 0x0800)
	    berrn_rom_load("1k.bin",    0x0800, 0x0800)
	berrn_rom_region("pal", 0x0020, 0)
	    berrn_rom_load("6l.bpr",    0x0000, 0x0020)
    berrn_rom_end

    GalaxianCore::GalaxianCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();

	main_proc = new BerrnZ80Processor(3072000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	video = new galaxianvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    if (irq_enable)
	    {
		main_proc->fire_nmi(true);
	    }

	    video->updatePixels();
	});
    }

    GalaxianCore::~GalaxianCore()
    {

    }

    bool GalaxianCore::initcore()
    {
	main_proc->init();
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);

	video->init();

	auto &scheduler = driver.get_scheduler();
	scheduler.add_device(main_cpu);
	vblank_timer->start(time_in_hz(60), true);
	driver.resize(256, 224, 2);
	return true;
    }

    void GalaxianCore::stopcore()
    {
	vblank_timer->stop();
	video->shutdown();
	main_rom.clear();
	main_proc->shutdown();
    }

    void GalaxianCore::runcore()
    {
	driver.run_scheduler();
    }

    uint8_t GalaxianCore::readCPU8(uint16_t addr)
    {
	uint8_t data = 0xFF;

	if (addr < 0x4000)
	{
	    data = main_rom.at(addr);
	}
	else if (inRange(addr, 0x4000, 0x4800))
	{
	    data = main_ram.at(addr & 0x3FF);
	}
	else if (inRange(addr, 0x5000, 0x5800))
	{
	    data = video->readVRAM(addr);
	}
	else if (inRange(addr, 0x5800, 0x6000))
	{
	    data = video->readORAM(addr);
	}
	else if (inRange(addr, 0x6000, 0x6800))
	{
	    // IN0
	    data = 0x00;
	}
	else if (inRange(addr, 0x6800, 0x7000))
	{
	    // IN1
	    data = 0x00;
	}
	else if (inRange(addr, 0x7000, 0x7800))
	{
	    // IN2
	    data = 0x04;
	}
	else if (inRange(addr, 0x7800, 0x8000))
	{
	    // Watchdog timer (unimplemented)
	    data = 0xFF;
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void GalaxianCore::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x4000)
	{
	    return;
	}
	else if (inRange(addr, 0x4000, 0x4800))
	{
	    main_ram.at(addr & 0x3FF) = data;
	}
	else if (inRange(addr, 0x5000, 0x5800))
	{
	    video->writeVRAM(addr, data);
	}
	else if (inRange(addr, 0x5800, 0x6000))
	{
	    video->writeORAM(addr, data);
	}
	else if (inRange(addr, 0x6000, 0x6800))
	{
	    int io_select = (addr & 7);

	    if (io_select >= 4)
	    {
		cout << "Writing value of " << hex << int(data) << " to Galaxian lower register of " << dec << int(io_select) << endl;
	    }
	}
	else if (inRange(addr, 0x6800, 0x7000))
	{
	    int io_select = (addr & 7);
	    cout << "Writing value of " << hex << int(data) << " to Galaxian upper register of " << dec << int(io_select) << endl;
	}
	else if (inRange(addr, 0x7000, 0x7800))
	{
	    writeIOUpper((addr & 7), testbit(data, 0));
	}
	else if (inRange(addr, 0x7800, 0x8000))
	{
	    cout << "Writing value of " << hex << int(data) << " to Galaxian $7800 register" << endl;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    void GalaxianCore::writeIOUpper(int reg, bool line)
    {
	switch (reg)
	{
	    case 1:
	    {
		irq_enable = line;

		if (!irq_enable)
		{
		    main_proc->fire_nmi(false);
		}
	    }
	    break;
	    case 4: video->writeIO(0, line); break;
	    case 6:
	    {
		if (line)
		{
		    cout << "X-flip enabled" << endl;
		}
		else
		{
		    cout << "X-flip disabled" << endl;
		}
	    }
	    break;
	    case 7:
	    {
		if (line)
		{
		    cout << "Y-flip enabled" << endl;
		}
		else
		{
		    cout << "Y-flip disabled" << endl;
		}
	    }
	    break;
	    default: break;
	}
    }

    drivergalaxian::drivergalaxian()
    {
	core = new GalaxianCore(*this);
    }

    drivergalaxian::~drivergalaxian()
    {

    }

    string drivergalaxian::drivername()
    {
	return "galaxian";
    }

    uint32_t drivergalaxian::get_flags()
    {
	return berrn_rot_90;
    }

    bool drivergalaxian::drvinit()
    {
	if (!loadROM(berrn_rom_name(galaxian)))
	{
	    return false;
	}

	return core->initcore();
    }

    void drivergalaxian::drvshutdown()
    {
	core->stopcore();
    }
  
    void drivergalaxian::drvrun()
    {
	core->runcore();
    }

    void drivergalaxian::keychanged(BerrnInput key, bool is_pressed)
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