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

#include <tmnt.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    // TODO: Load in other ROMs in this set as well
    // (currently only the M68K ROMs are included)
    berrn_rom_start(tmnt)
	berrn_rom_region("maincpu", 0x60000, 0)
	    berrn_rom_load16_byte("963-x23.j17", 0x00000, 0x20000)
	    berrn_rom_load16_byte("963-x24.k17", 0x00001, 0x20000)
	    berrn_rom_load16_byte("963-x21.j15", 0x40000, 0x10000)
	    berrn_rom_load16_byte("963-x22.k15", 0x40001, 0x10000)
	berrn_rom_region("k052109", 0x100000, 0)
	    berrn_rom_load32_word("963a28.h27", 0x000000, 0x80000)
	    berrn_rom_load32_word("963a29.k27", 0x000002, 0x80000)
    berrn_rom_end

    TMNTM68K::TMNTM68K(berrndriver &drv, TMNTCore &core) : driver(drv), main_core(core)
    {

    }

    TMNTM68K::~TMNTM68K()
    {

    }

    void TMNTM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void TMNTM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t TMNTM68K::readCPU16(bool upper, bool lower, uint32_t addr)
    {
	uint16_t data = 0;
	if (addr < 0x60000)
	{
	    if (upper)
	    {
		data |= (main_rom.at(addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_rom.at(addr + 1);
	    }
	}
	else if (inRange(addr, 0x60000, 0x64000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);
	    if (upper)
	    {
		data |= (main_ram.at(ram_addr) << 8);
	    }

	    if (lower)
	    {
		data |= main_ram.at(ram_addr + 1);
	    }
	}
	else if (inRange(addr, 0x80000, 0x81000))
	{
	    if (lower)
	    {
		data = main_core.readPalette((addr >> 1));
	    }
	}
	else if (addr == 0xA0018)
	{
	    // DSW3
	    if (lower)
	    {
		data = 0xFF;
	    }
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    data = main_core.readk052109_noA12(upper, lower, (addr >> 1));
	}
	else if (inRange(addr, 0x140000, 0x140008))
	{
	    cout << "K051937 register read" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    data = 0;
	}
	else if (inRange(addr, 0x140400, 0x140800))
	{
	    cout << "K051960 register read" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    data = 0;
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    void TMNTM68K::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x60000)
	{
	    return;
	}
	else if (inRange(addr, 0x60000, 0x64000))
	{
	    uint32_t ram_addr = (addr & 0x3FFF);
	    if (upper)
	    {
		main_ram.at(ram_addr) = (data >> 8);
	    }

	    if (lower)
	    {
		main_ram.at(ram_addr + 1) = (data & 0xFF);
	    }
	}
	else if (inRange(addr, 0x80000, 0x81000))
	{
	    if (lower)
	    {
		main_core.writePalette((addr >> 1), data);
	    }
	}
	else if (addr == 0xA0000)
	{
	    if (lower)
	    {
		main_core.write0A0000(data);
	    }
	}
	else if (addr == 0xA0010)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (inRange(addr, 0x100000, 0x108000))
	{
	    main_core.writek052109_noA12(upper, lower, (addr >> 1), data);
	}
	else if (addr == 0x10E800)
	{
	    return; // ???
	}
	else if (inRange(addr, 0x140000, 0x140008))
	{
	    cout << "K051937 register write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	}
	else if (inRange(addr, 0x140400, 0x1407FF))
	{
	    cout << "K051960 register write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	}
	else
	{
	    BerrnInterface::writeCPU16(upper, lower, addr, data);
	}
    }

    TMNTCore::TMNTCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();

	main_inter = new TMNTM68K(driver, *this);
	main_proc = new BerrnM68KProcessor(8000000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	palette = new BerrnPaletteXBGR555(1024);
    }

    TMNTCore::~TMNTCore()
    {

    }

    bool TMNTCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();

	main_inter->init();
	main_proc->init();
	scheduler.add_device(main_cpu);
	return true;
    }

    void TMNTCore::stop_core()
    {
	main_inter->shutdown();
	main_proc->shutdown();
    }

    void TMNTCore::run_core()
    {
	driver.run_scheduler();
    }

    uint8_t TMNTCore::readPalette(uint32_t addr)
    {
	addr &= 0x7FF;
	return palette->read8(addr);
    }

    void TMNTCore::writePalette(uint32_t addr, uint8_t data)
    {
	addr &= 0x7FF;
	palette->write8(addr, data);
    }

    void TMNTCore::write0A0000(uint16_t data)
    {
	if (testbit(data, 3))
	{
	    cout << "Bit 3 enabled" << endl;
	}
	else
	{
	    cout << "Bit 3 disabled" << endl;
	}

	if (testbit(data, 5))
	{
	    cout << "IRQ enabled" << endl;
	}
	else
	{
	    cout << "IRQ disabled" << endl;
	}

	if (testbit(data, 7))
	{
	    cout << "Setting K052109 RM/RD line" << endl;
	}
	else
	{
	    cout << "Clearing K052109 RM/RD line" << endl;
	}
    }

    uint16_t TMNTCore::readk052109_noA12(bool upper, bool lower, uint32_t addr)
    {
	addr = (((addr & 0x3000) >> 1) | (addr & 0x7FF));
	/*
	cout << "K052109 read" << endl;
	cout << "Upper: " << dec << int(upper) << endl;
	cout << "Lower: " << dec << int(lower) << endl;
	cout << "Address: " << hex << int(addr) << endl;
	cout << endl;
	*/
	return 0;
    }

    void TMNTCore::writek052109_noA12(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	addr = (((addr & 0x3000) >> 1) | (addr & 0x7FF));
	/*
	cout << "K052109 write" << endl;
	cout << "Upper: " << dec << int(upper) << endl;
	cout << "Lower: " << dec << int(lower) << endl;
	cout << "Address: " << hex << int(addr) << endl;
	cout << "Data: " << hex << int(data) << endl;
	cout << endl;
	*/
    }

    drivertmnt::drivertmnt()
    {
	core = new TMNTCore(*this);
    }

    drivertmnt::~drivertmnt()
    {

    }

    string drivertmnt::drivername()
    {
	return "tmnt";
    }

    bool drivertmnt::drvinit()
    {
	if (!loadROM(berrn_rom_name(tmnt)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivertmnt::drvshutdown()
    {
	core->stop_core();
    }
  
    void drivertmnt::drvrun()
    {
	core->run_core();
    }

    void drivertmnt::keychanged(BerrnInput key, bool is_pressed)
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