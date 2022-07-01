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

#include "galaga.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(galaga)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("gg1_1b.3p", 0x0000, 0x1000)
	    berrn_rom_load("gg1_2b.3m", 0x1000, 0x1000)
	    berrn_rom_load("gg1_3.2m",  0x2000, 0x1000)
	    berrn_rom_load("gg1_4b.2l", 0x3000, 0x1000)
	berrn_rom_region("auxcpu", 0x10000, 0)
	    berrn_rom_load("gg1_5b.3f", 0x0000, 0x1000)
	berrn_rom_region("soundcpu", 0x10000, 0)
	    berrn_rom_load("gg1_7b.2c", 0x0000, 0x1000)
	berrn_rom_region("gfx1", 0x1000, 0)
	    berrn_rom_load("gg1_9.4l", 0x0000, 0x1000)
	berrn_rom_region("color", 0x0020, 0)
	    berrn_rom_load("prom-5.5n", 0x0000, 0x0020)
	berrn_rom_region("tilepal", 0x0100, 0)
	    berrn_rom_load("prom-4.2n", 0x0000, 0x0100)
    berrn_rom_end

    GalagaInterface::GalagaInterface(berrndriver &drv, GalagaCore &core) : driver(drv), shared_core(core)
    {
	
    }

    GalagaInterface::~GalagaInterface()
    {

    }

    void GalagaInterface::init(string tag)
    {
	main_rom = driver.get_rom_region(tag);
    }

    void GalagaInterface::shutdown()
    {
	main_rom.clear();
    }

    uint8_t GalagaInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x4000)
	{
	    data = main_rom.at(addr);
	}
	else
	{
	    data = shared_core.readByte(addr);
	}

	return data;
    }

    void GalagaInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x4000)
	{
	    return;
	}
	else
	{
	    shared_core.writeByte(addr, data);
	}
    }

    GalagaCore::GalagaCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new GalagaInterface(driver, *this);
	aux_inter = new GalagaInterface(driver, *this);
	sound_inter = new GalagaInterface(driver, *this);
	main_cpu = new BerrnZ80CPU(driver, 3072000, *main_inter);
	aux_cpu = new BerrnZ80CPU(driver, 3072000, *aux_inter);
	sound_cpu = new BerrnZ80CPU(driver, 3072000, *sound_inter);

	auto chip_select = [&](int addr, bool line) -> void
	{
	    addr &= 3;

	    switch (addr)
	    {
		case 0:
		{
		    if (line)
		    {
			cout << "Asserting Namco 51xx chip select line..." << endl;
		    }
		    else
		    {
			cout << "Clearing Namco 51xx chip select line..." << endl;
		    }
		}
		break;
		default: break;
	    }
	};

	n06xx = new namco06xx(driver, 48000);
	n06xx->set_chip_select(chip_select);

	video = new galagavideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    video->updatePixels();
	});
    }

    GalagaCore::~GalagaCore()
    {

    }

    uint8_t GalagaCore::readByte(uint16_t addr)
    {
	uint8_t data = 0;

	if (inRange(addr, 0x6800, 0x6808))
	{
	    int dsw_offs = (addr & 7);
	    bool bit0 = testbit(dsw_b, dsw_offs);
	    bool bit1 = testbit(dsw_a, dsw_offs);
	    data = ((bit1 << 1) | bit0);
	}
	else if (addr == 0x7100)
	{
	    data = n06xx->readControl();
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    data = video->readVRAM(addr);
	}
	else if (inRange(addr, 0x8800, 0x8C00))
	{
	    data = readRAM(0, addr);
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    data = readRAM(1, addr);
	}
	else if (inRange(addr, 0x9800, 0x9C00))
	{
	    data = readRAM(2, addr);
	}
	else
	{
	    cout << "Reading from Galaga address of " << hex << int(addr) << endl;
	    exit(0);
	}

	return data;
    }

    void GalagaCore::writeByte(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0x6820, 0x6828))
	{
	    writeIO(addr, testbit(data, 0));
	}
	else if (addr == 0x6830)
	{
	    return;
	}
	else if (inRange(addr, 0x7000, 0x7100))
	{
	    n06xx->writeData(data);
	}
	else if (addr == 0x7100)
	{
	    n06xx->writeControl(data);
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    video->writeVRAM(addr, data);
	}
	else if (inRange(addr, 0x8800, 0x8C00))
	{
	    writeRAM(0, addr, data);
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    writeRAM(1, addr, data);
	}
	else if (inRange(addr, 0x9800, 0x9C00))
	{
	    writeRAM(2, addr, data);
	}
	else if (inRange(addr, 0xA000, 0xA008))
	{
	    return;
	}
	else
	{
	    cout << "Writing value of " << hex << int(data) << " to Galaga address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    void GalagaCore::writeIO(int addr, bool line)
    {
	addr &= 7;

	switch (addr)
	{
	    case 0:
	    {
		if (line)
		{
		    cout << "Asserting line 0..." << endl;
		}
		else
		{
		    cout << "Clearing line 0..." << endl;
		}
	    }
	    break;
	    case 1:
	    {
		if (line)
		{
		    cout << "Asserting line 1..." << endl;
		}
		else
		{
		    cout << "Clearing line 1..." << endl;
		}
	    }
	    break;
	    case 2:
	    {
		if (line)
		{
		    cout << "Asserting line 2..." << endl;
		}
		else
		{
		    cout << "Clearing line 2..." << endl;
		}
	    }
	    break;
	    case 3:
	    {
		if (line)
		{
		    cout << "Asserting line 3..." << endl;
		}
		else
		{
		    cout << "Clearing line 3..." << endl;
		}

		aux_cpu->set_reset_line(!line);
		sound_cpu->set_reset_line(!line);	
	    }
	    break;
	}
    }

    uint8_t GalagaCore::readRAM(int bank, uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x3FF;
	bank &= 3;

	if (bank == 3)
	{
	    return data;
	}

	data = main_ram[bank][addr];
	return data;
    }

    void GalagaCore::writeRAM(int bank, uint16_t addr, uint8_t data)
    {
	addr &= 0x3FF;
	bank &= 3;

	if (bank == 3)
	{
	    return;
	}

	main_ram[bank][addr] = data;
    }

    bool GalagaCore::init_core()
    {
	for (int i = 0; i < 3; i++)
	{
	    main_ram.at(i).fill(0);
	}

	auto &scheduler = driver.get_scheduler();
	scheduler.set_quantum(time_in_hz(6000));
	main_inter->init("maincpu");
	main_cpu->init();
	aux_inter->init("auxcpu");
	aux_cpu->init();
	sound_inter->init("soundcpu");
	sound_cpu->init();
	scheduler.add_device(main_cpu);
	scheduler.add_device(aux_cpu);
	scheduler.add_device(sound_cpu);
	n06xx->set_maincpu(main_cpu);
	vblank_timer->start(time_in_hz(60), true);
	video->init();

	for (int i = 0; i < 8; i++)
	{
	    writeIO(i, false);
	}

	dsw_a = 0xF7;
	dsw_b = 0x97;

	driver.resize(288, 224, 2);
	return true;
    }

    void GalagaCore::stop_core()
    {
	vblank_timer->stop();
	video->shutdown();
	sound_cpu->shutdown();
	sound_inter->shutdown();
	aux_cpu->shutdown();
	aux_inter->shutdown();
	main_cpu->shutdown();
	main_inter->shutdown();
    }

    void GalagaCore::run_core()
    {
	driver.run_scheduler();
    }

    drivergalaga::drivergalaga()
    {
	core = new GalagaCore(*this);
    }

    drivergalaga::~drivergalaga()
    {

    }

    string drivergalaga::drivername()
    {
	return "galaga";
    }

    uint32_t drivergalaga::get_flags()
    {
	return berrn_rot_90;
    }

    bool drivergalaga::drvinit()
    {
	if (!loadROM(berrn_rom_name(galaga)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivergalaga::drvshutdown()
    {
	core->stop_core();
    }
  
    void drivergalaga::drvrun()
    {
	core->run_core();
    }

    void drivergalaga::keychanged(BerrnInput key, bool is_pressed)
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