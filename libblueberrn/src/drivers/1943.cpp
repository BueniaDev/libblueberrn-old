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

#include "1943.h"
using namespace berrn;

// 1943: The Battle of Midway - Capcom (WIP)
//
// TODO:
// Finish implementing video system
// Implement sound system
// Implement LLE of security MCU


namespace berrn
{
    berrn_rom_start(1943u)
	berrn_rom_region("maincpu", 0x30000, 0)
	    berrn_rom_load("bmu01c.12d", 0x00000, 0x08000)
	    berrn_rom_load("bmu02c.13d", 0x10000, 0x10000)
	    berrn_rom_load("bmu03c.14d", 0x20000, 0x10000)
	berrn_rom_region("mcu", 0x1000, 0)
	    berrn_rom_load("bm.7k", 0x0000, 0x1000)
	berrn_rom_region("gfx2", 0x40000, 0)
	    berrn_rom_load("bm15.10f", 0x00000, 0x8000)
	    berrn_rom_load("bm16.11f", 0x08000, 0x8000)
	    berrn_rom_load("bm17.12f", 0x10000, 0x8000)
	    berrn_rom_load("bm18.14f", 0x18000, 0x8000)
	    berrn_rom_load("bm19.10j", 0x20000, 0x8000)
	    berrn_rom_load("bm20.11j", 0x28000, 0x8000)
	    berrn_rom_load("bm21.12j", 0x30000, 0x8000)
	    berrn_rom_load("bm22.14j", 0x38000, 0x8000)
	berrn_rom_region("gfx3", 0x10000, 0)
	    berrn_rom_load("bm24.14k", 0x0000, 0x8000)
	    berrn_rom_load("bm25.14l", 0x8000, 0x8000)
	berrn_rom_region("tilerom", 0x10000, 0)
	    berrn_rom_load("bm14.5f", 0x0000, 0x8000)
	    berrn_rom_load("bm23.8k", 0x8000, 0x8000)
	berrn_rom_region("colorproms", 0x0300, 0)
	    berrn_rom_load("bm1.12a", 0x0000, 0x0100)
	    berrn_rom_load("bm2.13a", 0x0100, 0x0100)
	    berrn_rom_load("bm3.14a", 0x0200, 0x0100)
	berrn_rom_region("fgproms", 0x0200, 0)
	    berrn_rom_load("bm10.7l",  0x0000, 0x0100)
	    berrn_rom_load("bm9.6l",   0x0100, 0x0100)
	berrn_rom_region("bgproms", 0x0200, 0)
	    berrn_rom_load("bm12.12m", 0x0000, 0x0100)
	    berrn_rom_load("bm11.12l", 0x0100, 0x0100)
    berrn_rom_end

    Berrn1943Main::Berrn1943Main(berrndriver &drv, Berrn1943Core &core) : driver(drv), main_core(core)
    {

    }

    Berrn1943Main::~Berrn1943Main()
    {

    }

    void Berrn1943Main::init()
    {
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
    }

    void Berrn1943Main::shutdown()
    {
	main_rom.clear();
    }

    uint8_t Berrn1943Main::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;
	if (addr < 0x8000)
	{
	    data = main_rom.at(addr);
	}
	else if (inRange(addr, 0x8000, 0xC000))
	{
	    uint32_t rom_addr = (0x10000 + ((addr - 0x8000) + (current_rom_bank * 0x4000)));
	    data = main_rom.at(rom_addr);
	}
	else if (addr == 0xC000)
	{
	    // SYSTEM
	    data = 0xFF;
	}
	else if (addr == 0xC003)
	{
	    // DSWA
	    data = 0xF8;
	}
	else if (addr == 0xC004)
	{
	    // DSWB
	    data = 0xFF;
	}
	else if (addr == 0xC007)
	{
	    data = main_core.readSecurity();
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    data = main_ram.at(addr & 0xFFF);
	}
	else if (inRange(addr, 0xF000, 0x10000))
	{
	    data = sprite_ram.at(addr & 0xFFF);
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void Berrn1943Main::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0xC000)
	{
	    return;
	}
	else if (addr == 0xC800)
	{
	    // TODO: Implement sound support
	    return;
	}
	else if (addr == 0xC804)
	{
	    current_rom_bank = ((data >> 2) & 0x7);

	    string char_str = testbit(data, 7) ? "Enabling" : "Disabling";
	    cout << char_str << " characters..." << endl;
	}
	else if (addr == 0xC806)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (addr == 0xC807)
	{
	    main_core.writeSecurity(data);
	}
	else if (inRange(addr, 0xD000, 0xD400))
	{
	    video_ram.at(addr & 0x3FF) = data;
	}
	else if (inRange(addr, 0xD400, 0xD800))
	{
	    color_ram.at(addr & 0x3FF) = data;
	}
	else if (addr == 0xD800)
	{
	    main_core.writeIO(1, data);
	}
	else if (addr == 0xD801)
	{
	    main_core.writeIO(2, data);
	}
	else if (addr == 0xD802)
	{
	    main_core.writeIO(3, data);
	}
	else if (addr == 0xD803)
	{
	    main_core.writeIO(4, data);
	}
	else if (addr == 0xD804)
	{
	    main_core.writeIO(5, data);
	}
	else if (addr == 0xD806)
	{
	    string obj_str = testbit(data, 6) ? "Enabling" : "Disabling";
	    cout << obj_str << " sprites..." << endl;

	    main_core.writeIO(6, data);
	}
	else if (addr == 0xD808)
	{
	    return;
	}
	else if (addr == 0xD868)
	{
	    return;
	}
	else if (addr == 0xD888)
	{
	    return;
	}
	else if (addr == 0xD8A8)
	{
	    return;
	}
	else if (inRange(addr, 0xE000, 0xF000))
	{
	    main_ram.at(addr & 0xFFF) = data;
	}
	else if (inRange(addr, 0xF000, 0x10000))
	{
	    sprite_ram.at(addr & 0xFFF) = data;
	}
	else
	{
	    BerrnInterface::writeCPU8(addr, data);
	}
    }

    Berrn1943Core::Berrn1943Core(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_inter = new Berrn1943Main(driver, *this);
	main_cpu = new BerrnZ80CPU(driver, 6000000, *main_inter);
	video = new berrn1943video(driver);
    }

    Berrn1943Core::~Berrn1943Core()
    {

    }

    bool Berrn1943Core::init_core()
    {
	auto screen = driver.get_screen();
	auto &scheduler = driver.get_scheduler();
	screen->set_raw(6000000, 384, 128, 0, 262, 22, 246);
	screen->set_vblank_callback([&](bool line) -> void
	{
	    run_vblank(line);
	});
	main_inter->init();
	main_cpu->init();
	video->init();
	screen->init();
	scheduler.add_device(main_cpu);
	driver.resize(256, 224, 2);
	return true;
    }

    void Berrn1943Core::stop_core()
    {
	auto screen = driver.get_screen();
	screen->shutdown();
	video->shutdown();
	main_inter->shutdown();
	main_cpu->shutdown();
    }

    void Berrn1943Core::run_core()
    {
	driver.run_scheduler();
    }

    void Berrn1943Core::run_vblank(bool line)
    {
	if (!line)
	{
	    return;
	}

	video->updatePixels();
	main_cpu->fireInterrupt8(0xFF);
    }

    uint8_t Berrn1943Core::readSecurity()
    {
	return security_val;
    }

    void Berrn1943Core::writeSecurity(uint8_t data)
    {
	auto security_index = security_hle.find(data);

	if (security_index == security_hle.end())
	{
	    security_val = 0;
	}
	else
	{
	    security_val = security_index->second;
	}
    }

    void Berrn1943Core::writeIO(int addr, uint8_t data)
    {
	switch (addr)
	{
	    case 1: video->writeBGScrollX(false, data); break;
	    case 2: video->writeBGScrollX(true, data); break;
	    case 3: video->writeBGScrollY(data); break;
	    case 4: video->writeBG2Scroll(false, data); break;
	    case 5: video->writeBG2Scroll(true, data); break;
	    case 6: video->writeD806(data); break;
	    default: break;
	}
    }

    driver1943u::driver1943u()
    {
	core = new Berrn1943Core(*this);
    }

    driver1943u::~driver1943u()
    {

    }

    string driver1943u::drivername()
    {
	return "1943u";
    }

    string driver1943u::parentname()
    {
	return "1943";
    }

    uint32_t driver1943u::get_flags()
    {
	return berrn_rot_270;
    }

    bool driver1943u::drvinit()
    {
	if (!loadROM(berrn_rom_name(1943u)))
	{
	    return false;
	}

	return core->init_core();
    }

    void driver1943u::drvshutdown()
    {
	core->stop_core();
    }
  
    void driver1943u::drvrun()
    {
	core->run_core();
    }

    void driver1943u::keychanged(BerrnInput key, bool is_pressed)
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