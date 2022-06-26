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

#include "konmedal.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(shuriboy)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("gs-341-b01.13g", 0x00000, 0x10000)
	berrn_rom_region("k052109", 0x40000, 0)
	    berrn_rom_load32_byte("341-a03.2h", 0x0000, 0x10000)
	    berrn_rom_load32_byte("341-a04.4h", 0x0001, 0x10000)
	    berrn_rom_load32_byte("341-a05.5h", 0x0002, 0x10000)
	    berrn_rom_load32_byte("341-a06.7h", 0x0003, 0x10000)
	berrn_rom_region("upd", 0x200000, 0)
	    berrn_rom_load("341-a02.13c", 0x00000, 0x20000)
	berrn_rom_region("proms", 0x400, 0)
	    berrn_rom_load("342_a07.2d", 0x0000, 0x0100)
	    berrn_rom_load("342_a08.3d", 0x0100, 0x0100)
	    berrn_rom_load("342_a09.4d", 0x0200, 0x0100)
	    berrn_rom_load("341_a10.3e", 0x0300, 0x0100)
    berrn_rom_end

    berrn_rom_start(mariorou)
	berrn_rom_region("maincpu", 0x10000, 0)
	    berrn_rom_load("111_d01.3g.27c512", 0x00000, 0x10000)
	berrn_rom_region("k052109", 0x40000, 0)
	    berrn_rom_load32_byte("111_c03.2h.27c256", 0x0000, 0x8000)
	    berrn_rom_load32_byte("111_c04.4h.27c256", 0x0001, 0x8000)
	    berrn_rom_load32_byte("111_c05.5h.27c256", 0x0002, 0x8000)
	    berrn_rom_load32_byte("111_c06.7h.27c256", 0x0003, 0x8000)
	berrn_rom_region("upd", 0x200000, 0)
	    berrn_rom_load("111_c02.13c.27c010", 0x00000, 0x20000)
	berrn_rom_region("proms", 0x400, berrn_rom_erase00)
	    berrn_rom_load("111_c07.2d.82s129", 0x0000, 0x0100)
	    berrn_rom_load("111_c08.3d.82s129", 0x0100, 0x0100)
	    berrn_rom_load("111_c09.4d.82s129", 0x0200, 0x0100)
	    berrn_rom_load("111_a10.3e.82s129", 0x0300, 0x0100)
    berrn_rom_end

    BerrnShuriBoy::BerrnShuriBoy(berrndriver &drv) : driver(drv)
    {
	auto &scheduler = driver.get_scheduler();
	main_cpu = new BerrnZ80CPU(driver, 8000000, *this);

	tile_video = new shuriboyvideo(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    updatePixels();
	    vblank_start_time = driver.get_scheduler().get_current_time();
	    vblank_timer->start(time_until_pos(240), false);
	});

	irq_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    int next_vpos = 0;

	    if (!is_first_time)
	    {
		int current_vpos = vpos();

		scanline_callback(current_vpos);

		if ((current_vpos + 1) < 256)
		{
		    next_vpos = (current_vpos + 1);
		}
	    }

	    is_first_time = false;

	    irq_timer->start(time_until_pos(next_vpos), false);
	});
    }

    BerrnShuriBoy::~BerrnShuriBoy()
    {

    }

    void BerrnShuriBoy::scanline_callback(int scanline)
    {
	if ((scanline == 240) && is_irq_enabled)
	{
	    main_cpu->fireInterrupt8();
	}

	if ((scanline == 255) && is_nmi_enabled)
	{
	    main_cpu->fireNMI();
	}
    }

    int64_t BerrnShuriBoy::time_until_pos(int vpos)
    {
	auto &scheduler = driver.get_scheduler();
	vpos = ((vpos + 16) % 256);

	int64_t scan_time = (time_in_hz(60) / 256);

	int64_t target_delta = (vpos * scan_time);

	int64_t current_delta = (scheduler.get_current_time() - vblank_start_time);

	while (target_delta <= current_delta)
	{
	    target_delta += time_in_hz(60);
	}

	return (target_delta - current_delta);
    }

    bool BerrnShuriBoy::init_core()
    {
	auto &scheduler = driver.get_scheduler();
	main_cpu->init();
	scheduler.add_device(main_cpu);
	main_rom = driver.get_rom_region("maincpu");
	main_ram.fill(0);
	scc_ram.fill(0);
	init_ram();
	current_scanline = 0;
	current_rom_bank = 0;
	tile_video->init();
	vblank_start_time = 0;
	vblank_timer->start(time_until_pos(240), false);
	is_first_time = true;
	irq_timer->start(0, false);
	in2_port = 0xEF;
	driver.resize(288, 224, 2);
	return true;
    }

    void BerrnShuriBoy::stop_core()
    {
	tile_video->shutdown();
	vblank_timer->stop();
	irq_timer->stop();
	main_rom.clear();
	main_cpu->shutdown();
    }

    void BerrnShuriBoy::run_core()
    {
	driver.run_scheduler();
    }

    void BerrnShuriBoy::key_changed(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnService:
	    {
		in2_port = changebit(in2_port, 1, !is_pressed);
	    }
	    break;
	    default: break;
	}
    }

    void BerrnShuriBoy::init_ram()
    {
	writeRAM(0x08, 0x01);
	writeRAM(0x09, 0x02);
	writeRAM(0x0E, 0x04);
	writeRAM(0x17, 0x08);
	writeRAM(0x18, 0x10);
	writeRAM(0x21, 0x20);
	writeRAM(0x50, 0x40);
	writeRAM(0x52, 0x80);
	writeRAM(0x53, 0x55);
	writeRAM(0x54, 0xAA);
	writeRAM(0x5D, 0x33);
    }

    uint8_t BerrnShuriBoy::readDIP(int bank)
    {
	uint8_t data = 0;

	switch (bank)
	{
	    // IN2
	    case 0: data = in2_port; break;
	    // IN1
	    case 1: data = 0x7F; break;
	    // DSW1
	    case 2: data = 0x87; break;
	    // DSW2
	    case 3: data = 0x40; break;
	    default: data = 0; break;
	}

	return data;
    }

    void BerrnShuriBoy::updatePixels()
    {
	tile_video->updatePixels();
    }

    int BerrnShuriBoy::vpos()
    {
	int64_t delta = (driver.get_scheduler().get_current_time() - vblank_start_time);

	int64_t pixel_time = (time_in_hz(60) / (512 * 256));
	int64_t scan_time = (time_in_hz(60) / 256);

	delta += (pixel_time / 2);

	int vpos = delta / scan_time;

	return ((vpos + 240) % 256);
    }

    uint8_t BerrnShuriBoy::readCPU8(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x8000)
	{
	    data = main_rom.at(addr);
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    data = main_ram.at(addr - 0x8000);
	}
	else if (inRange(addr, 0x8800, 0x8804))
	{
	    data = readDIP(addr - 0x8800);
	}
	else if (inRange(addr, 0x9800, 0x9900))
	{
	    uint16_t scc_addr = (addr - 0x9800);
	    // cout << "Reading from K051649 address of " << hex << int(scc_addr) << endl;
	    data = scc_ram.at(scc_addr);
	}
	else if (inRange(addr, 0xA000, 0xC000))
	{
	    uint32_t rom_addr = (0x8000 + ((addr - 0xA000) + (current_rom_bank * 0x2000)));
	    data = main_rom.at(rom_addr);
	}
	else if (addr == 0xDD00)
	{
	    data = ((is_irq_enabled << 2) | is_nmi_enabled);
	}
	else if (inRange(addr, 0xC000, 0x10000))
	{
	    uint16_t tile_addr = (addr - 0xC000);
	    data = tile_video->read(tile_addr);
	}
	else
	{
	    data = BerrnInterface::readCPU8(addr);
	}

	return data;
    }

    void BerrnShuriBoy::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x8000)
	{
	    return;
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    main_ram.at(addr - 0x8000) = data;
	}
	else if (addr == 0x8900)
	{
	    // cout << "Writing value of " << hex << int(data) << " to control 2 register" << endl;

	    if (testbit(data, 1))
	    {
		cout << "Asserting uPD7759 reset line..." << endl;
	    }
	    else
	    {
		cout << "Clearing uPD7759 reset line..." << endl;
	    }

	    if (testbit(data, 0))
	    {
		cout << "Asserting uPD7759 start line..." << endl;
	    }
	    else
	    {
		cout << "Clearing uPD7759 start line..." << endl;
	    }
	}
	else if (addr == 0x8A00)
	{
	    cout << "Writing value of " << hex << int(data) << " to medal control register" << endl;
	}
	else if (addr == 0x8B00)
	{
	    return; // Possible watchdog timer?
	}
	else if (addr == 0x8C00)
	{
	    tile_video->setRMRD(testbit(data, 6));
	    current_rom_bank = (data & 0x3);
	}
	else if (addr == 0x8D00)
	{
	    cout << "Writing value of " << hex << int(data) << " to uPD7759 port register" << endl;
	}
	else if (addr == 0x8E00)
	{
	    // cout << "Writing value of " << hex << int(data) << " to lamps register" << endl;
	    return;
	}
	else if (addr == 0x9000)
	{
	    return; // Writes alternate between 00 and 3F
	}
	else if (inRange(addr, 0x9800, 0x9900))
	{
	    uint16_t scc_addr = (addr - 0x9800);
	    // cout << "Writing value of " << hex << int(data) << " to K051649 address of " << hex << int(scc_addr) << endl;
	    scc_ram.at(scc_addr) = data;
	}
	else if (addr == 0xDD00)
	{
	    if (is_irq_enabled && !testbit(data, 2))
	    {
		main_cpu->clearInterrupt();
	    }

	    if (is_nmi_enabled && !testbit(data, 0))
	    {
		main_cpu->fireNMI(false);
	    }

	    is_irq_enabled = testbit(data, 2);
	    is_nmi_enabled = testbit(data, 0);
	}
	else if (inRange(addr, 0xC000, 0x10000))
	{
	    uint16_t tile_addr = (addr - 0xC000);
	    tile_video->write(tile_addr, data);
	}
    }

    BerrnMarioRoulette::BerrnMarioRoulette(berrndriver &drv) : BerrnShuriBoy(drv)
    {
	tile_video = new mariorouvideo(drv);
    }

    BerrnMarioRoulette::~BerrnMarioRoulette()
    {

    }

    void BerrnMarioRoulette::init_ram()
    {
	writeRAM(0x502, 0xFF);
	writeRAM(0x506, 0xAA);
	writeRAM(0x508, 0x55);
    }

    uint8_t BerrnMarioRoulette::readDIP(int bank)
    {
	uint8_t data = 0;
	switch (bank)
	{
	    case 2: data = 0xFF; break;
	    case 3: data = 0x78; break;
	    default: data = BerrnShuriBoy::readDIP(bank); break;
	}

	return data;
    }

    void BerrnMarioRoulette::updatePixels()
    {
	tile_video->updatePixels();
    }

    drivershuriboy::drivershuriboy()
    {
	core = new BerrnShuriBoy(*this);
    }

    drivershuriboy::~drivershuriboy()
    {

    }

    string drivershuriboy::drivername()
    {
	return "shuriboy";
    }

    bool drivershuriboy::drvinit()
    {
	if (!loadROM(berrn_rom_name(shuriboy)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivershuriboy::drvshutdown()
    {
	core->stop_core();
    }
  
    void drivershuriboy::drvrun()
    {
	core->run_core();
    }

    void drivershuriboy::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }

    drivermariorou::drivermariorou()
    {
	core = new BerrnMarioRoulette(*this);
    }

    drivermariorou::~drivermariorou()
    {

    }

    string drivermariorou::drivername()
    {
	return "mariorou";
    }

    bool drivermariorou::drvinit()
    {
	if (!loadROM(berrn_rom_name(mariorou)))
	{
	    return false;
	}

	return core->init_core();
    }

    void drivermariorou::drvshutdown()
    {
	core->stop_core();
    }
  
    void drivermariorou::drvrun()
    {
	core->run_core();
    }

    void drivermariorou::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }
};