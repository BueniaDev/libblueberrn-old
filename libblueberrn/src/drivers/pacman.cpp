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

#include <pacman.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    berrn_rom_start(pacman)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("pacman.6e", 0x0000, 0x1000)
	    berrn_rom_load("pacman.6f", 0x1000, 0x1000)
	    berrn_rom_load("pacman.6h", 0x2000, 0x1000)
	    berrn_rom_load("pacman.6j", 0x3000, 0x1000)
	berrn_rom_region("color", 0x0020, 0)
	    berrn_rom_load("82s123.7f", 0x0000, 0x0020)
	berrn_rom_region("pal", 0x0100, 0)
	    berrn_rom_load("82s126.4a", 0x0000, 0x0100)
	berrn_rom_region("gfx1", 0x1000, 0)
	    berrn_rom_load("pacman.5e", 0x0000, 0x1000)
	berrn_rom_region("gfx2", 0x1000, 0)
	    berrn_rom_load("pacman.5f", 0x0000, 0x1000)
    berrn_rom_end

    PacmanCore::PacmanCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_cpu = new BerrnZ80CPU(driver, 3072000, *this);

	video = new pacmanvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    if (is_irq_enabled)
	    {
		main_cpu->fireInterrupt();
	    }

	    video->updatePixels();
	});
    }

    PacmanCore::~PacmanCore()
    {

    }

    bool PacmanCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_cpu->init();
	scheduler.add_device(main_cpu);
	vblank_timer->start(time_in_hz(60), true);
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
	video->init();
	driver.resize(288, 224, 2);
	return true;
    }

    void PacmanCore::stop_core()
    {
	vblank_timer->stop();
	video->shutdown();
	main_cpu->shutdown();
	main_rom.clear();
    }

    void PacmanCore::run_core()
    {
	driver.run_scheduler();
    }

    uint8_t PacmanCore::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x7FFF;

	uint32_t mirror_addr = (addr & 0x5FFF);

	if (addr < 0x4000)
	{
	    data = main_rom.at(addr);
	}
	else if (inRange(mirror_addr, 0x4000, 0x4800))
	{
	    data = video->readVRAM(addr);
	}
	else if (inRange(mirror_addr, 0x4800, 0x4C00))
	{
	    data = 0xBF;
	}
	else if (inRange(mirror_addr, 0x4C00, 0x4FF0))
	{
	    data = main_ram.at((addr - 0x4C00));
	}
	else if (inRange(mirror_addr, 0x4FF0, 0x5000))
	{
	    data = video->readORAM(addr);
	}
	else if (inRange(mirror_addr, 0x5000, 0x6000))
	{
	    data = readIO(mirror_addr);
	}

	return data;
    }

    void PacmanCore::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FFF;

	uint32_t mirror_addr = (addr & 0x5FFF);

	if (addr < 0x4000)
	{
	    return;
	}
	else if (inRange(mirror_addr, 0x4000, 0x4800))
	{
	    video->writeVRAM(addr, data);
	}
	else if (inRange(mirror_addr, 0x4C00, 0x4FF0))
	{
	    main_ram.at((addr - 0x4C00)) = data;
	}
	else if (inRange(mirror_addr, 0x4FF0, 0x5000))
	{
	    video->writeORAM(0, addr, data);
	}
	else if (inRange(mirror_addr, 0x5000, 0x6000))
	{
	    writeIO(mirror_addr, data);
	}
    }

    void PacmanCore::portOut(uint16_t port, uint8_t data)
    {
	port &= 0xFF;

	if (port == 0)
	{
	    main_cpu->setIRQVector(data);
	}
    }

    uint8_t PacmanCore::readIO(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x50FF;

	if (inRange(addr, 0x5000, 0x5040))
	{
	    // IN0
	    data = 0xFF;
	}
	else if (inRange(addr, 0x5040, 0x5080))
	{
	    // IN1
	    data = 0xFF;
	}
	else if (inRange(addr, 0x5080, 0x50C0))
	{
	    // DSW1
	    data = 0xC9;
	}
	else if (inRange(addr, 0x50C0, 0x5100))
	{
	    // DSW2
	    data = 0xFF;
	}

	return data;
    }

    void PacmanCore::writeIO(uint16_t addr, uint8_t data)
    {
	addr &= 0x50FF;
	if (inRange(addr, 0x5000, 0x5040))
	{
	    writeLatch(addr, testbit(data, 0));
	}
	else if (inRange(addr, 0x5040, 0x5060))
	{
	    cout << "Writing value of " << hex << int(data) << " to Namco WSG3 device of " << hex << int(addr & 0x1F) << endl;
	}
	else if (inRange(addr, 0x5060, 0x5070))
	{
	    video->writeORAM(1, addr, data);
	}
	else if (inRange(addr, 0x50C0, 0x5100))
	{
	    // Watchdog reset (unimplemented)
	    return;
	}
    }

    void PacmanCore::writeLatch(int addr, bool line)
    {
	addr &= 7;

	switch (addr)
	{
	    case 0:
	    {
		is_irq_enabled = line;

		if (!line)
		{
		    main_cpu->clearInterrupt();
		}
	    }
	    break;
	    case 1:
	    {
		if (line)
		{
		    cout << "Sound enabled" << endl;
		}
		else
		{
		    cout << "Sound disabled" << endl;
		}
	    }
	    break;
	    case 3:
	    {
		if (line)
		{
		    cout << "Screen is flipped" << endl;
		}
		else
		{
		    cout << "Screen is normal" << endl;
		}
	    }
	    break;
	}
    }

    driverpacman::driverpacman()
    {
	core = new PacmanCore(*this);
    }

    driverpacman::~driverpacman()
    {

    }

    string driverpacman::drivername()
    {
	return "pacman";
    }

    string driverpacman::parentname()
    {
	return "puckman";
    }

    uint32_t driverpacman::get_flags()
    {
	return berrn_rot_90;
    }

    bool driverpacman::drvinit()
    {
	if (!loadROM(berrn_rom_name(pacman)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverpacman::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverpacman::drvrun()
    {
	core->run_core();
    }

    void driverpacman::keychanged(BerrnInput key, bool is_pressed)
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