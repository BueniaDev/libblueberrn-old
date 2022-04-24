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

#include <centipede.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    berrn_rom_start(centiped)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("136001-407.d1",  0x2000, 0x0800)
	    berrn_rom_load("136001-408.e1",  0x2800, 0x0800)
	    berrn_rom_load("136001-409.fh1", 0x3000, 0x0800)
	    berrn_rom_load("136001-410.j1",  0x3800, 0x0800)
    berrn_rom_end

    BerrnCentipedeBase::BerrnCentipedeBase(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_proc = new Berrn6502Processor(1512000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    current_scanline += 1;

	    if (testbit(current_scanline, 4))
	    {
		bool is_irq_line = testbit((current_scanline - 1), 5);
		main_proc->fire_interrupt(is_irq_line);
	    }

	    if (current_scanline == 256)
	    {
		current_scanline = 0;
	    }
	});
    }

    BerrnCentipedeBase::~BerrnCentipedeBase()
    {

    }

    bool BerrnCentipedeBase::initcore()
    {
	auto &scheduler = driver.get_scheduler();
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
	temp_vram.fill(0);
	main_proc->fire_interrupt(false);
	earom_control_write(0x00);
	main_proc->init();
	scheduler.add_device(main_cpu);
	// VBlank time / 256
	irq_timer->start(65, true);
	driver.resize(640, 480, 1);
	return true;
    }

    void BerrnCentipedeBase::stopcore()
    {
	main_rom.clear();
	main_proc->shutdown();
    }

    void BerrnCentipedeBase::runcore()
    {
	driver.run_scheduler();
    }

    uint8_t BerrnCentipedeBase::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x3FFF;

	if (inRange(addr, 0x0000, 0x0400))
	{
	    data = main_ram.at(addr);
	}
	else if (inRange(addr, 0x0400, 0x0800))
	{
	    data = temp_vram.at(addr - 0x400);
	}
	else if (addr == 0x0800)
	{
	    // DSW1
	    data = 0x54;
	}
	else if (addr == 0x0801)
	{
	    // DSW2
	    data = 0x02;
	}
	else if (addr == 0x0C00)
	{
	    // TODO: IN0 reads
	    data = 0xBF;
	}
	else if (inRange(addr, 0x1700, 0x1740))
	{
	    data = earom_read();
	}
	else if (inRange(addr, 0x2000, 0x4000))
	{
	    data = main_rom.at(addr);
	}
	else if (isValidAddr(addr, false))
	{
	    data = readMem(addr);
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void BerrnCentipedeBase::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FFF;

	if (inRange(addr, 0x0000, 0x0400))
	{
	    main_ram.at(addr) = data;
	}
	else if (inRange(addr, 0x0400, 0x0800))
	{
	    temp_vram.at(addr - 0x400) = data;
	}
	else if (inRange(addr, 0x1600, 0x1640))
	{
	    earom_write(addr, data);
	}
	else if (addr == 0x1680)
	{
	    earom_control_write(data);
	}
	else if (inRange(addr, 0x1C00, 0x1C08))
	{
	    writeLatch(addr, data);
	}
	else if (inRange(addr, 0x2000, 0x4000))
	{
	    return;
	}
	else if (isValidAddr(addr, true))
	{
	    writeMem(addr, data);
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    uint8_t BerrnCentipedeBase::earom_read()
    {
	// EAROM read
	return earom.data();
    }

    void BerrnCentipedeBase::earom_write(int addr, uint8_t data)
    {
	// EAROM write
	earom.set_address(addr & 0x3F);
	earom.set_data(data);
    }

    void BerrnCentipedeBase::earom_control_write(uint8_t data)
    {
	// EAROM control write
	// CK = DB0, C1 = /DB1, C2 = DB2, CS1 = DB3, /CS2 = GND
	earom.set_control(testbit(data, 3), true, !testbit(data, 1), testbit(data, 2));
	earom.set_clk(testbit(data, 0));
    }

    bool BerrnCentipedeBase::isValidAddr(uint16_t addr, bool is_write)
    {
	(void)addr;
	(void)is_write;
	return false;
    }

    uint8_t BerrnCentipedeBase::readMem(uint16_t addr)
    {
	(void)addr;
	return 0;
    }

    void BerrnCentipedeBase::writeMem(uint16_t addr, uint8_t data)
    {
	(void)addr;
	(void)data;
	return;
    }

    void BerrnCentipedeBase::writeLatch(int addr, uint8_t data)
    {
	addr &= 7;
	bool line = testbit(data, 7);

	switch (addr)
	{
	    case 0: break;
	    case 1: break;
	    case 2: break;
	    case 3: break;
	    case 4: break;
	    default: writeLatchUpper(addr, line); break;
	}
    }

    void BerrnCentipedeBase::writeLatchUpper(int addr, bool line)
    {
	(void)addr;
	(void)line;
	return;
    }

    BerrnCentipede::BerrnCentipede(berrndriver &drv) : BerrnCentipedeBase(drv)
    {

    }

    BerrnCentipede::~BerrnCentipede()
    {

    }

    bool BerrnCentipede::isValidAddr(uint16_t addr, bool is_write)
    {
	(void)is_write;
	addr &= 0x3FFF;
	return inRange(addr, 0x1000, 0x1010);
    }

    uint8_t BerrnCentipede::readMem(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x3FFF;

	if (inRange(addr, 0x1000, 0x1010))
	{
	    cout << "Reading value from Pokey address of " << hex << int(addr & 0xF) << endl;
	    data = 0;
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void BerrnCentipede::writeMem(uint16_t addr, uint8_t data)
    {
	addr &= 0x3FFF;
	if (inRange(addr, 0x1000, 0x1010))
	{
	    cout << "Writing value of " << hex << int(data) << " to Pokey address of " << hex << int(addr & 0xF) << endl;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    void BerrnCentipede::writeLatchUpper(int addr, bool line)
    {
	switch (addr)
	{
	    case 7:
	    {
		if (line)
		{
		    cout << "Screen flipped" << endl;
		}
		else
		{
		    cout << "Screen is normal" << endl;
		}
	    }
	    break;
	    default: break;
	}
    }

    drivercentiped::drivercentiped()
    {
	core = new BerrnCentipede(*this);
    }

    drivercentiped::~drivercentiped()
    {

    }

    string drivercentiped::drivername()
    {
	return "centiped";
    }

    bool drivercentiped::drvinit()
    {
	if (!loadROM(berrn_rom_name(centiped)))
	{
	    return false;
	}

	return core->initcore();
    }

    void drivercentiped::drvshutdown()
    {
	core->stopcore();
    }
  
    void drivercentiped::drvrun()
    {
	core->runcore();
    }

    void drivercentiped::keychanged(BerrnInput key, bool is_pressed)
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