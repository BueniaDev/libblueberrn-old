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

#include "system1.h"
using namespace berrn;

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

    Sys1MainInterface::Sys1MainInterface()
    {

    }

    Sys1MainInterface::~Sys1MainInterface()
    {

    }

    void Sys1MainInterface::init()
    {
	vram.fill(0);
	mainram.fill(0);
    }

    void Sys1MainInterface::shutdown()
    {
	return;
    }

    uint8_t Sys1MainInterface::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void Sys1MainInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0xC000)
	{
	    return;
	}
	else if (addr < 0xD000)
	{
	    mainram[(addr & 0xFFF)] = data;
	}
	else if (addr < 0xE000)
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
	else if (addr < 0xF000)
	{
	    vram[(addr & 0xFFF)] = data;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    uint8_t Sys1MainInterface::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    void Sys1MainInterface::portOut(uint16_t port, uint8_t data)
    {
	if (outputcb)
	{
	    outputcb((port & 0x1F), data);
	}
    }

    uint8_t Sys1MainInterface::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x8000)
	{
	    data = gamerom.at(addr);
	}
	else if (addr < 0xC000)
	{
	    cout << "Reading byte from address of " << hex << int(addr) << endl;
	    exit(0);
	}
	else if (addr < 0xD000)
	{
	    data = mainram[(addr & 0xFFF)];
	}
	else if (addr < 0xE000)
	{
	    cout << "Reading byte from address of " << hex << int(addr) << endl;
	    exit(0);
	}
	else if (addr < 0xF000)
	{
	    data = vram[(addr & 0xFFF)];
	}
	else
	{
	    cout << "Reading byte from address of " << hex << int(addr) << endl;
	    exit(0);
	}

	return data;
    }

    SegaSys1PPI::SegaSys1PPI()
    {
	scheduler.set_interleave(100);
	// 20 mHz is not really the actual clock rate of the main Z80
	// (see notes for details)
	main_proc = new BerrnZ80Processor(20000000, main_inter);
	main_proc->set_prescalers(5, 2); // See notes for details
	main_cpu = new BerrnCPU(scheduler, *main_proc);
    }

    SegaSys1PPI::~SegaSys1PPI()
    {

    }

    void SegaSys1PPI::init()
    {
	scheduler.reset();
	scheduler.add_device(main_cpu);
	main_proc->init();
	auto outputfunc = bind(&SegaSys1PPI::writeOutput, this, _1, _2);
	main_inter.setoutputcallback(outputfunc);
	main_inter.init();
	return;
    }

    void SegaSys1PPI::shutdown()
    {
	main_inter.shutdown();
	main_proc->shutdown();
	scheduler.remove_device(main_cpu);
    }

    void SegaSys1PPI::run()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = time_in_hz(60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}
    }

    void SegaSys1PPI::writeOutput(int addr, uint8_t data)
    {
	if ((addr >= 0x14) && (addr <= 0x17))
	{
	    main_ppi.write((addr & 0x3), data);
	}
    }

    driverwboy2u::driverwboy2u()
    {

    }

    driverwboy2u::~driverwboy2u()
    {

    }

    string driverwboy2u::drivername()
    {
	return "wboy2u";
    }

    bool driverwboy2u::hasdriverROMs()
    {
	return true;
    }

    void driverwboy2u::loadROMs()
    {
	loadROM("ic129_02.bin", 0x0000, 0x2000, core_sys1.get_main_rom());
	loadROM("ic130_03.bin", 0x2000, 0x2000, core_sys1.get_main_rom());
	loadROM("ic131_04.bin", 0x4000, 0x2000, core_sys1.get_main_rom());
	loadROM("ic132_05.bin", 0x6000, 0x2000, core_sys1.get_main_rom());
	loadROM("epr-7591.133", 0x8000, 0x2000, core_sys1.get_main_rom());
	loadROM("epr-7591.133", 0xA000, 0x2000, core_sys1.get_main_rom());
    }

    bool driverwboy2u::drvinit()
    {
	loadROMs();
	core_sys1.init();
	resize(512, 224, 2);
	return isallfilesloaded();
    }

    void driverwboy2u::drvshutdown()
    {
	core_sys1.shutdown();
    }
  
    void driverwboy2u::drvrun()
    {
	core_sys1.run();
	fillrect(0, 0, 512, 224, red());
    }

    void driverwboy2u::keychanged(BerrnInput key, bool is_pressed)
    {
	return;
    }
};