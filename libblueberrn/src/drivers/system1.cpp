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

#include <system1.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    // Blueberry's Notes:
    //
    // How the Sega System 1's main CPU is clocked:
    //
    // Basically, to generate the main CPU's clock,
    // the 20MHz master clock is being divided by 6 when an opcode
    // is being fetched (i.e. when the Z80's /M1 signal is low), 
    // and by 5 during all other clock cycles.
    // For a more detailed explanation, see the notes in MAME's implementation:
    // https://github.com/mamedev/mame/blob/master/src/mame/drivers/system1.cpp
    //
    // To account for this, support was added in the BeeZ80 engine
    // (which is providing Zilog Z80 emulation for libblueberrn) for customizable
    // pre-scalers for both original Z80 cycles and opcode fetch cycles.

    berrn_rom_start(wboy2u)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("ic129_02.bin", 0x0000, 0x2000)
	    berrn_rom_load("ic130_03.bin", 0x2000, 0x2000)
	    berrn_rom_load("ic131_04.bin", 0x4000, 0x2000)
	    berrn_rom_load("ic132_05.bin", 0x6000, 0x2000)
	    berrn_rom_load("epr-7591.133", 0x8000, 0x2000)
	    berrn_rom_load("epr-7592.134", 0xA000, 0x2000)
    berrn_rom_end

    Sys1MainInterface::Sys1MainInterface(berrndriver &drv, SegaSystem1 &core) : driver(drv), main_core(core)
    {

    }

    Sys1MainInterface::~Sys1MainInterface()
    {

    }

    void Sys1MainInterface::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void Sys1MainInterface::shutdown()
    {
	main_rom.clear();
    }

    uint8_t Sys1MainInterface::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0xC000)
	{
	    data = main_rom.at(addr);
	}
	else if (addr < 0xD000)
	{
	    data = main_ram.at((addr & 0xFFF));
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void Sys1MainInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0xC000)
	{
	    return;
	}
	else if (addr < 0xD000)
	{
	    main_ram.at((addr & 0xFFF)) = data;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    void Sys1MainInterface::portOut(uint16_t port, uint8_t data)
    {
	main_core.portOut((port & 0x1F), data);
    }

    SegaSystem1::SegaSystem1(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	scheduler.set_quantum(time_in_hz(6000));

	main_inter = new Sys1MainInterface(driver, *this);

	// 20 mHz is not really the actual clock rate of the main Z80
	// (see notes for details)
	main_proc = new BerrnZ80Processor(20000000, *main_inter);
	main_proc->set_prescalers(5, 2); // See notes for details
	main_cpu = new BerrnCPU(scheduler, *main_proc);
    }

    SegaSystem1::~SegaSystem1()
    {

    }

    bool SegaSystem1::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	scheduler.add_device(main_cpu);
	main_inter->init();
	main_proc->init();
	return true;
    }

    void SegaSystem1::stop_core()
    {
	main_inter->shutdown();
	main_proc->shutdown();
    }

    void SegaSystem1::run_core()
    {
	driver.run_scheduler();
    }

    void SegaSystem1::portOut(uint16_t addr, uint8_t data)
    {
	cout << "Writing value of " << hex << int(data) << " to Sega System 1 port of " << hex << int(addr) << endl;
	exit(0);
    }

    SegaSys1PPI::SegaSys1PPI(berrndriver &drv) : SegaSystem1(drv)
    {

    }

    SegaSys1PPI::~SegaSys1PPI()
    {

    }

    bool SegaSys1PPI::init_core()
    {
	main_ppi.init();
	main_ppi.set_out_portb_callback([&](uint8_t data) -> void
	{
	    cout << "Writing value of " << hex << int(data) << " to video mode register" << endl;
	});

	return SegaSystem1::init_core();
    }

    void SegaSys1PPI::stop_core()
    {
	main_ppi.shutdown();
	SegaSystem1::stop_core();
    }

    void SegaSys1PPI::portOut(uint16_t addr, uint8_t data)
    {
	switch (addr)
	{
	    case 0x14: main_ppi.write(0, data); break;
	    case 0x15: main_ppi.write(1, data); break;
	    case 0x16: main_ppi.write(2, data); break;
	    case 0x17: main_ppi.write(3, data); break;
	    default: break;
	}
    }

    driverwboy2u::driverwboy2u()
    {
	core = new SegaSys1PPI(*this);
    }

    driverwboy2u::~driverwboy2u()
    {

    }

    string driverwboy2u::drivername()
    {
	return "wboy2u";
    }

    string driverwboy2u::parentname()
    {
	return "wboy";
    }

    bool driverwboy2u::drvinit()
    {
	if (!loadROM(berrn_rom_name(wboy2u)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverwboy2u::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverwboy2u::drvrun()
    {
	core->run_core();
    }

    void driverwboy2u::keychanged(BerrnInput key, bool is_pressed)
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