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

#include "pacman.h"
using namespace berrn;

namespace berrn
{
    berrn_rom_start(pacman)
	berrn_rom_region("cpu", 0x4000)
	    berrn_rom_load("pacman.6e", 0x0000, 0x1000)
            berrn_rom_load("pacman.6f", 0x1000, 0x1000)
            berrn_rom_load("pacman.6h", 0x2000, 0x1000)
	    berrn_rom_load("pacman.6j", 0x3000, 0x1000)
	berrn_rom_region("color", 0x0020)
	    berrn_rom_load("82s123.7f", 0x0000, 0x0020)
	berrn_rom_region("pal", 0x0100)
	    berrn_rom_load("82s126.4a", 0x0000, 0x0100)
	berrn_rom_region("gfx1", 0x1000)
	    berrn_rom_load("pacman.5e", 0x0000, 0x1000)
	berrn_rom_region("gfx2", 0x1000)
	    berrn_rom_load("pacman.5f", 0x0000, 0x1000)
	berrn_rom_region("namco", 0x0200)
	    berrn_rom_load("82s126.1m", 0x0000, 0x0100)
	    berrn_rom_load("82s126.3m", 0x0100, 0x0100)
    berrn_rom_end

    PacmanInterface::PacmanInterface(berrndriver &drv) : driver(drv)
    {
	video_core = new pacmanvideo(driver);

	main_proc = new BerrnZ80Processor(3072000, *this);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	audio_chip = new wsg3device(driver);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    update_pixels();
	});

	interrupt_timer = new BerrnTimer("Interrupt", scheduler, [&](int64_t param, int64_t)
	{
	    if (param == 1)
	    {
		interrupt_timer->start(16500, true);
	    }

	    if (is_irq_enabled)
	    {
		main_proc->fire_interrupt8(irq_vector);
	    }
	});

	sound_timer = new BerrnTimer("Sound", scheduler, [&](int64_t, int64_t)
	{
	    auto samples = audio_chip->fetch_samples();
	    driver.addMonoSample(samples[0]);
	    driver.outputAudio();
	});
    }

    PacmanInterface::~PacmanInterface()
    {

    }

    bool PacmanInterface::init_core()
    {
	scheduler.reset();
	scheduler.add_device(main_cpu);
	main_proc->init();
	video_core->init();

	port0_val = 0xFF;
	port1_val = 0xFF;

	vblank_timer->start(16500, true);
	interrupt_timer->start(14000, false, 1);
	sound_timer->start(time_in_hz(driver.getSampleRate()), true);

	for (int i = 0; i < 8; i++)
	{
	    writeIO(i, 0);
	}

	ram.fill(0);

	driver.resize(224, 288, 2);
	game_rom = driver.get_rom_region("cpu");
	audio_chip->init();
	audio_chip->set_sample_rates(96000, driver.getSampleRate());
	return true;
    }

    void PacmanInterface::shutdown_core()
    {
	audio_chip->shutdown();
	video_core->shutdown();
	main_proc->shutdown();
	interrupt_timer->stop();
	vblank_timer->stop();
	sound_timer->stop();
	scheduler.shutdown();
    }

    void PacmanInterface::run_core()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = 16500;

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}
    }

    void PacmanInterface::key_changed(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		port0_val = changebit(port0_val, 5, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		port1_val = changebit(port1_val, 5, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnUpP1:
	    {
		port0_val = changebit(port0_val, 0, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		port0_val = changebit(port0_val, 1, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		port0_val = changebit(port0_val, 2, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnDownP1:
	    {
		port0_val = changebit(port0_val, 3, !is_pressed);
	    }
	    break;
	    default: break;
	}
    }

    void PacmanInterface::update_pixels()
    {
	video_core->updatePixels();
    }

    uint8_t PacmanInterface::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void PacmanInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	writeByte(addr, data);
    }

    uint8_t PacmanInterface::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    void PacmanInterface::portOut(uint16_t port, uint8_t data)
    {
	writePort(port, data);
    }

    uint8_t PacmanInterface::readByte(uint16_t addr)
    {
	addr &= 0x7FFF;
	uint8_t data = 0;

	if (addr < 0x4000)
	{
	    data = game_rom.at(addr);
	}
	else
	{
	    data = readUpper(addr);
	}

	return data;
    }

    void PacmanInterface::writeByte(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FFF;

	if (addr < 0x4000)
	{
	    return;
	}
	else
	{
	    writeUpper(addr, data);
	}
    }

    uint8_t PacmanInterface::readUpper(uint16_t addr)
    {
	uint8_t data = 0;
	addr &= 0x5FFF;

	uint16_t addr_io = (addr & 0x50FF);

	if (inRange(addr, 0x4000, 0x4800))
	{
	    data = video_core->readByte(addr);
	}
	else if (inRange(addr, 0x4800, 0x4C00))
	{
	    // Return value of reading the bus with no devices enabled
	    // (value derived from MAME sources)
	    data = 0xBF;
	}
	else if (inRange(addr, 0x4C00, 0x4FF0))
	{
	    data = ram[(addr & 0x3FF)];
	}
	else if (inRange(addr, 0x4FF0, 0x5000))
	{
	    data = video_core->readSprites(addr);
	}
	else if (inRange(addr_io, 0x5000, 0x5040))
	{
	    data = port0_val;
	}
	else if (inRange(addr_io, 0x5040, 0x5080))
	{
	    data = port1_val;
	}
	else if (inRange(addr_io, 0x5080, 0x50C0))
	{
	    // IN2 port values
	    //
	    // Bits 0-1 - Number of credits per game
	    // 0 = None (free play)
	    // 1 = 1 credit per game
	    // 2 = 1 credit per 2 games
	    // 3 = 2 credits per game
	    //
	    // Bits 2-3 - Number of lives per game:
	    // 0 = 1 life
	    // 1 = 2 lives
	    // 2 = 3 lives
	    // 3 = 5 lives
	    //
	    // Bits 4-5 - Bonus life at:
	    // 0 = 10000 points
	    // 1 = 15000 points
	    // 2 = 20000 points
	    // 3 = N/A
	    //
	    // Bit 6 - Difficulty:
	    // 0 = Hard
	    // 1 = Normal
	    //
	    // Bit 7 - Ghost names:
	    // 0 = Alternate (BBBBBBBB, DDDDDDDD, FFFFFFFF, and HHHHHHHH)
	    // 1 = Normal (Blinky, Pinky, Inky, and Clyde)

	    data = 0xC9;
	}
	else
	{
	    cout << "Reading byte from address of " << hex << int(addr) << endl;
	    exit(0);
	    data = 0;
	}

	return data;
    }

    void PacmanInterface::writeUpper(uint16_t addr, uint8_t data)
    {
	addr &= 0x5FFF;

	uint16_t addr_io = (addr & 0x50FF);

	if (inRange(addr, 0x4000, 0x4800))
	{
	    video_core->writeByte(addr, data);
	}
	else if (inRange(addr, 0x4800, 0x4C00))
	{
	    return;
	}
	else if (inRange(addr, 0x4C00, 0x4FF0))
	{
	    ram[(addr & 0x3FF)] = data;
	}
	else if (inRange(addr, 0x4FF0, 0x5000))
	{
	    video_core->writeSprites(addr, data);
	}
	else if (inRange(addr_io, 0x5000, 0x5040))
	{
	    writeIO(addr_io, data);
	}
	else if (inRange(addr_io, 0x5040, 0x5060))
	{
	    audio_chip->write_reg((addr_io & 0x1F), data);
	}
	else if (inRange(addr_io, 0x5060, 0x5070))
	{
	    video_core->writeSpritePos(addr, data);
	}
	else if (inRange(addr_io, 0x5070, 0x50C0))
	{
	    return;
	}
	else if (inRange(addr_io, 0x50C0, 0x5100))
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else
	{
	    cout << "Writing byte of " << hex << int(data) << " to address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    void PacmanInterface::writeIO(int addr, uint8_t data)
    {
	addr &= 7;

	bool val = testbit(data, 0);

	switch (addr)
	{
	    case 0:
	    {
		is_irq_enabled = val;

		if (!val)
		{
		    main_proc->fire_interrupt8(irq_vector, false);
		}
	    }
	    break;
	    case 1:
	    {
		audio_chip->set_sound_enabled(val);
	    }
	    break;
	    default: break;
	}
    }

    void PacmanInterface::writePort(uint16_t port, uint8_t data)
    {
	port &= 0xFF;

	if (port == 0)
	{
	    main_proc->set_irq_vector(data);
	    irq_vector = data;
	}
    }

    driverpacman::driverpacman()
    {
	inter = new PacmanInterface(*this);
    }

    driverpacman::~driverpacman()
    {

    }

    string driverpacman::drivername()
    {
	return "pacman";
    }

    bool driverpacman::hasdriverROMs()
    {
	return true;
    }

    bool driverpacman::drvinit()
    {
	if (!loadROM(berrn_rom_name(pacman)))
	{
	    return false;
	}

	return inter->init_core();
    }

    void driverpacman::drvshutdown()
    {
	inter->shutdown_core();
    }
  
    void driverpacman::drvrun()
    {
	inter->run_core();
    }

    float driverpacman::get_framerate()
    {
	return (16000.0 / 132.0 / 2.0); // Framerate is 60.606060 Hz
    }

    void driverpacman::keychanged(BerrnInput key, bool is_pressed)
    {
	inter->key_changed(key, is_pressed);
    }
};