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

#include <rastan.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    berrn_rom_start(rastan)
	berrn_rom_region("maincpu", 0x60000, 0)
	    berrn_rom_load16_byte("b04-38.19",  0x00000, 0x10000)
	    berrn_rom_load16_byte("b04-37.7",   0x00001, 0x10000)
	    berrn_rom_load16_byte("b04-40.20",  0x20000, 0x10000)
	    berrn_rom_load16_byte("b04-39.8",   0x20001, 0x10000)
	    berrn_rom_load16_byte("b04-42.21",  0x40000, 0x10000)
	    berrn_rom_load16_byte("b04-43-1.9", 0x40001, 0x10000)
	berrn_rom_region("pc080sn", 0x080000, 0)
	    berrn_rom_load16_byte("b04-01.40",  0x00000, 0x20000)
	    berrn_rom_load16_byte("b04-02.67",  0x00001, 0x20000)
	    berrn_rom_load16_byte("b04-03.39",  0x40000, 0x20000)
	    berrn_rom_load16_byte("b04-04.66",  0x40001, 0x20000)
    berrn_rom_end

    RastanM68K::RastanM68K(berrndriver &drv, RastanCore &core) : driver(drv), main_core(core)
    {

    }

    RastanM68K::~RastanM68K()
    {

    }

    void RastanM68K::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void RastanM68K::shutdown()
    {
	main_rom.clear();
    }

    uint16_t RastanM68K::readCPU16(bool upper, bool lower, uint32_t addr)
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
	else if (inRange(addr, 0x10C000, 0x110000))
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
	else if (inRange(addr, 0x200000, 0x201000))
	{
	    /*
	    cout << "Reading palette RAM..." << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    */
	    data = 0x0000;
	}
	else if (addr == 0x390004)
	{
	    // SPECIAL
	    if (lower)
	    {
		data = 0x8F;
	    }
	}
	else if (addr == 0x390006)
	{
	    // SYSTEM
	    if (lower)
	    {
		data = 0x1F;
	    }
	}
	else if (addr == 0x390008)
	{
	    // DSWA
	    if (lower)
	    {
		data = 0xFE;
	    }
	}
	else if (addr == 0x39000A)
	{
	    // DSWB
	    if (lower)
	    {
		data = 0xFF;
	    }
	}
	else if (inRange(addr, 0xC00000, 0xC10000))
	{
	    /*
	    cout << "Taito PC080SN word read" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    */
	    data = main_core.readPC080SN(0, upper, lower, addr);
	}
	else if (inRange(addr, 0xD00000, 0xD04000))
	{
	    /*
	    cout << "Taito PC090OJ word read" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << endl;
	    */
	    data = 0x0000;
	}
	else
	{
	    data = BerrnInterface::readCPU16(upper, lower, addr);
	}

	return data;
    }

    void RastanM68K::writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	if (addr < 0x60000)
	{
	    return;
	}
	else if (inRange(addr, 0x10C000, 0x110000))
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
	else if (inRange(addr, 0x200000, 0x201000))
	{
	    /*
	    cout << "Writing to palette RAM..." << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    return;
	}
	else if (addr == 0x350008)
	{
	    return;
	}
	else if (addr == 0x380000)
	{
	    /*
	    cout << "Sprite control write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	}
	else if (addr == 0x3C0000)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (addr == 0x3E0000)
	{
	    if (!lower)
	    {
		// Even byte writes are ignored
		return;
	    }

	    // cout << "Writing value of " << hex << int((data & 0xFF)) << " to Taito PC060HA master port" << endl;
	}
	else if (addr == 0x3E0002)
	{
	    if (!lower)
	    {
		// Even byte writes are ignored
		return;
	    }

	    // cout << "Writing value of " << hex << int((data & 0xFF)) << " to Taito PC060HA master communication port" << endl;
	}
	else if (inRange(addr, 0xC00000, 0xC10000))
	{
	    /*
	    cout << "Taito PC080SN word write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    main_core.writePC080SN(0, upper, lower, addr, data);
	}
	else if (inRange(addr, 0xC20000, 0xC20004))
	{
	    /*
	    cout << "Taito PC080SN y-scroll word write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    main_core.writePC080SN(1, upper, lower, addr, data);
	}
	else if (inRange(addr, 0xC40000, 0xC40004))
	{
	    /*
	    cout << "Taito PC080SN x-scroll word write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    main_core.writePC080SN(2, upper, lower, addr, data);
	}
	else if (inRange(addr, 0xC50000, 0xC50004))
	{
	    /*
	    cout << "Taito PC080SN control word write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	    main_core.writePC080SN(3, upper, lower, addr, data);
	}
	else if (inRange(addr, 0xD00000, 0xD04000))
	{
	    /*
	    cout << "Taito PC090OJ write" << endl;
	    cout << "Upper: " << dec << int(upper) << endl;
	    cout << "Lower: " << dec << int(lower) << endl;
	    cout << "Address: " << hex << int(addr) << endl;
	    cout << "Data: " << hex << int(data) << endl;
	    cout << endl;
	    */
	}
	else
	{
	    BerrnInterface::writeCPU16(upper, lower, addr, data);
	}
    }

    RastanCore::RastanCore(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();

	main_inter = new RastanM68K(driver, *this);

	main_cpu = new BerrnM68KCPU(driver, 8000000, *main_inter);

	tile_video = new rastanvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    main_cpu->fireInterruptLevel(5);
	    tile_video->updatePixels();
	});
    }

    RastanCore::~RastanCore()
    {

    }

    bool RastanCore::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	scheduler.set_quantum(time_in_hz(600));
	main_inter->init();
	main_cpu->init();
	tile_video->init();
	scheduler.add_device(main_cpu);
	vblank_timer->start(time_in_hz(60), true);
	// driver.resize(512, 512, 1);
	driver.resize(320, 240, 2);

	return true;
    }

    void RastanCore::stop_core()
    {
	vblank_timer->stop();
	tile_video->shutdown();
	main_inter->shutdown();
	main_cpu->shutdown();
    }

    void RastanCore::run_core()
    {
	driver.run_scheduler();
    }

    uint16_t RastanCore::readPC080SN(int bank, bool upper, bool lower, uint32_t addr)
    {
	return tile_video->readPC080SN(bank, upper, lower, addr);
    }

    void RastanCore::writePC080SN(int bank, bool upper, bool lower, uint32_t addr, uint16_t data)
    {
	tile_video->writePC080SN(bank, upper, lower, addr, data);
    }

    driverrastan::driverrastan()
    {
	core = new RastanCore(*this);
    }

    driverrastan::~driverrastan()
    {

    }

    string driverrastan::drivername()
    {
	return "rastan";
    }

    bool driverrastan::drvinit()
    {
	if (!loadROM(berrn_rom_name(rastan)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driverrastan::drvshutdown()
    {
	core->stop_core();
    }
  
    void driverrastan::drvrun()
    {
	core->run_core();
    }

    void driverrastan::keychanged(BerrnInput key, bool is_pressed)
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