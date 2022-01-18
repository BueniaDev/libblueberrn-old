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
	berrn_rom_region("maincpu", 0x4000)
	    berrn_rom_load("gg1_1b.3p", 0x0000, 0x1000)
	    berrn_rom_load("gg1_2b.3m", 0x1000, 0x1000)
	    berrn_rom_load("gg1_3.2m", 0x2000, 0x1000)
	    berrn_rom_load("gg1_4b.2l", 0x3000, 0x1000)
	berrn_rom_region("auxcpu", 0x4000)
	    berrn_rom_load("gg1_5b.3f", 0x0000, 0x1000)
	berrn_rom_region("soundcpu", 0x4000)
	    berrn_rom_load("gg1_7b.2c", 0x0000, 0x1000)
	berrn_rom_region("gfx1", 0x1000)
	    berrn_rom_load("gg1_9.4l", 0x0000, 0x1000)
	berrn_rom_region("gfx2", 0x2000)
	    berrn_rom_load("gg1_11.4d", 0x0000, 0x1000)
	    berrn_rom_load("gg1_10.4f", 0x1000, 0x1000)
	berrn_rom_region("color", 0x0020)
	    berrn_rom_load("prom-5.5n", 0x0000, 0x0020)
	berrn_rom_region("tilepal", 0x0100)
	    berrn_rom_load("prom-4.2n", 0x0000, 0x0100)
	berrn_rom_region("spritepal", 0x0100)
	    berrn_rom_load("prom-3.1c", 0x0000, 0x0100)
	berrn_rom_region("namco", 0x0200)
	    berrn_rom_load("prom-1.1d", 0x0000, 0x0100)
	    berrn_rom_load("prom-2.5c", 0x0100, 0x0100)
    berrn_rom_end

    GalagaInterface::GalagaInterface(GalagaCore &core) : parent_core(core)
    {

    }

    GalagaInterface::~GalagaInterface()
    {

    }

    uint8_t GalagaInterface::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	if (inRange(addr, 0x4000, 0x9C00))
	{
	    data = parent_core.readByte(addr);
	}
	else
	{
	    cout << "Reading value from address of " << hex << int(addr) << endl;
	    exit(0);
	    data = 0;
	}

	return data;
    }

    void GalagaInterface::writeByte(uint16_t addr, uint8_t data)
    {
	if (inRange(addr, 0x4000, 0xA008))
	{
	    parent_core.writeByte(addr, data);
	}
	else
	{
	    cout << "Writing value of " << hex << int(data) << " to address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    GalagaCPUInterface::GalagaCPUInterface(string tag_str, berrndriver &drv, GalagaInterface &inter) : tag(tag_str), driver(drv), interface(inter)
    {

    }

    GalagaCPUInterface::~GalagaCPUInterface()
    {

    }

    void GalagaCPUInterface::init()
    {
	cpu_rom = driver.get_rom_region(tag);
    }

    void GalagaCPUInterface::shutdown()
    {
	cpu_rom.clear();
    }

    uint8_t GalagaCPUInterface::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void GalagaCPUInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	writeByte(addr, data);
    }

    uint8_t GalagaCPUInterface::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t GalagaCPUInterface::readByte(uint16_t addr)
    {
	uint8_t data = 0;

	if (addr < 0x4000)
	{
	    data = cpu_rom.at(addr);
	}
	else
	{
	    data = interface.readByte(addr);
	}

	return data;
    }

    void GalagaCPUInterface::writeByte(uint16_t addr, uint8_t data)
    {
	if (addr < 0x4000)
	{
	    return;
	}
	else
	{
	    interface.writeByte(addr, data);
	}
    }

    GalagaCore::GalagaCore(berrndriver &drv) : driver(drv)
    {
	scheduler.set_interleave(100);

	shared_inter = new GalagaInterface(*this);

	main_inter = new GalagaCPUInterface("maincpu", drv, *shared_inter);
	aux_inter = new GalagaCPUInterface("auxcpu", drv, *shared_inter);
	sound_inter = new GalagaCPUInterface("soundcpu", drv, *shared_inter);

	main_proc = new BerrnZ80Processor(3072000, *main_inter);
	aux_proc = new BerrnZ80Processor(3072000, *aux_inter);
	sound_proc = new BerrnZ80Processor(3072000, *sound_inter);

	main_cpu = new BerrnCPU(scheduler, *main_proc);
	aux_cpu = new BerrnCPU(scheduler, *aux_proc);
	sound_cpu = new BerrnCPU(scheduler, *sound_proc);

	video_core = new galagavideo(driver);

	wsg3_device = new wsg3device(driver);

	interrupt_timer = new BerrnTimer("IRQ", scheduler, [&](int64_t, int64_t)
	{
	    fire_main_irq();
	    fire_aux_irq();
	    fire_51xx_irq();
	});

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t)
	{
	    vblank_start_time = scheduler.get_current_time();
	    video_core->updatePixels();
	});

	sound_nmi_timer = new BerrnTimer("Sound IRQ", scheduler, [&](int64_t param, int64_t)
	{
	    int scanline = int(param);

	    if (sound_irq_enabled)
	    {
		sound_proc->fire_nmi();
	    }

	    scanline += 128;

	    if (scanline >= 272)
	    {
		scanline = 64;
	    }

	    sound_nmi_timer->start(time_until_scanline(scanline), false, scanline);
	});

	sound_timer = new BerrnTimer("Sound", scheduler, [&](int64_t, int64_t)
	{
	    auto samples = wsg3_device->fetch_samples();
	    driver.addMonoSample(samples[0]);
	    driver.outputAudio();
	});

	berrncbwrite8 output_io = [&](int addr, uint8_t data) -> void
	{
	    (void)addr;
	    (void)data;
	    return;
	};

	namco_51xx = new namco51xx(driver, scheduler, 1536000);
	namco_51xx->set_input_callback(bind(&GalagaCore::read_io_ports, this, _1));
	namco_51xx->set_output_callback(output_io);

	namco_54xx = new namco54xx(driver, scheduler, 1536000);

	namco_06xx = new namco06xx(*main_cpu, 48000);
	namco_06xx->set_chipsel_callback(bind(&GalagaCore::chip_select_galaga, this, _1, _2));
	namco_06xx->set_rw_callback(bind(&GalagaCore::rw_galaga, this, _1, _2));
	namco_06xx->set_read_callback(bind(&GalagaCore::read_galaga, this, _1));
	namco_06xx->set_write_callback(bind(&GalagaCore::write_galaga, this, _1, _2));
    }

    GalagaCore::~GalagaCore()
    {

    }

    bool GalagaCore::init_core()
    {
	scheduler.reset();
	scheduler.add_device(main_cpu);
	scheduler.add_device(aux_cpu);
	scheduler.add_device(sound_cpu);

	if (!namco_51xx->init())
	{
	    scheduler.shutdown();
	    return false;
	}

	/*
	if (!namco_54xx->init())
	{
	    scheduler.shutdown();
	    return false;
	}
	*/

	main_inter->init();
	aux_inter->init();
	sound_inter->init();

	main_proc->init();
	aux_proc->init();
	sound_proc->init();

	wsg3_device->init();
	wsg3_device->set_sound_enabled(true);
	wsg3_device->set_sample_rates(96000, driver.getSampleRate());

	for (int i = 0; i < 8; i++)
	{
	    write_latch(i, 0);
	}

	main_ram1.fill(0);
	main_ram2.fill(0);
	main_ram3.fill(0);

	video_core->init();

	dsw_a = 0xF7;
	dsw_b = 0x97;

	port0_val = 0xFF;
	port1_val = 0xFF;

	vblank_start_time = 0;

	driver.resize(224, 288, 2);
	vblank_timer->start(time_in_hz(60), true);
	interrupt_timer->start(time_in_hz(60), true);
	sound_nmi_timer->start(time_until_scanline(64), false, 64);
	sound_timer->start(time_in_hz(driver.getSampleRate()), true);
	return true;
    }

    void GalagaCore::shutdown_core()
    {
	// namco_54xx->shutdown();
	namco_51xx->shutdown();
	wsg3_device->shutdown();
	sound_timer->stop();
	sound_nmi_timer->stop();
	interrupt_timer->stop();
	vblank_timer->stop();
	main_proc->shutdown();
	aux_proc->shutdown();
	sound_proc->shutdown();
	video_core->shutdown();
	scheduler.shutdown();
    }

    void GalagaCore::run_core()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = time_in_hz(60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}
    }

    void GalagaCore::key_changed(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		port1_val = changebit(port1_val, 4, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		port1_val = changebit(port1_val, 2, !is_pressed);

		if (is_pressed)
		{
		    namco_51xx->dump = true;
		}
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		port0_val = changebit(port0_val, 3, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		port0_val = changebit(port0_val, 1, !is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnFireP1:
	    {
		port1_val = changebit(port1_val, 0, !is_pressed);
	    }
	    break;
	    default: break;
	}
    }

    void GalagaCore::fire_main_irq()
    {
	if (main_irq_enabled && !main_cpu->is_suspended(SuspendReason::Reset))
	{
	    main_proc->fire_interrupt8(0xFF);
	}
    }

    void GalagaCore::fire_aux_irq()
    {
	if (aux_irq_enabled && !aux_cpu->is_suspended(SuspendReason::Reset))
	{
	    aux_proc->fire_interrupt8(0xFF);
	}
    }

    void GalagaCore::fire_51xx_irq()
    {
	namco_51xx->vblank();
    }

    int64_t GalagaCore::time_until_scanline(int scanline)
    {
	int64_t vpos = ((scanline + 20) % 264);
	int64_t target_delta = (vpos * (time_in_hz(60) / 264));
	int64_t current_delta = (scheduler.get_current_time() - vblank_start_time);

	while (target_delta <= current_delta)
	{
	    target_delta += time_in_hz(60);
	}

	return (target_delta - current_delta); // us
    }

    uint8_t GalagaCore::readByte(uint16_t addr)
    {
	uint8_t data = 0;
	uint16_t ram_addr = (addr & 0x3FF);

	if (inRange(addr, 0x6800, 0x6808))
	{
	    int dsw_offs = (addr & 7);
	    int bit0 = testbit(dsw_b, dsw_offs);
	    int bit1 = testbit(dsw_a, dsw_offs);
	    data = ((bit1 << 1) | bit0);
	}
	else if (inRange(addr, 0x7000, 0x7100))
	{
	    data = namco_06xx->read_data();
	}
	else if (addr == 0x7100)
	{
	    data = namco_06xx->read_control();
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    data = video_core->readByte(addr);
	}
	else if (inRange(addr, 0x8800, 0x8C00))
	{
	    data = main_ram1.at(ram_addr);
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    data = main_ram2.at(ram_addr);
	}
	else if (inRange(addr, 0x9800, 0x9C00))
	{
	    data = main_ram3.at(ram_addr);
	}
	else
	{
	    cout << "Reading value from Galaga address of " << hex << int(addr) << endl;
	    exit(0);
	    data = 0;
	}

	return data;
    }

    void GalagaCore::writeByte(uint16_t addr, uint8_t data)
    {
	uint16_t ram_addr = (addr & 0x3FF);

	if (inRange(addr, 0x6800, 0x6820))
	{
	    wsg3_device->write_reg((addr & 0x1F), data);
	}
	else if (inRange(addr, 0x6820, 0x6828))
	{
	    write_latch(addr, data);
	}
	else if (addr == 0x6830)
	{
	    // Watchdog timer (unimplemented)
	    return;
	}
	else if (inRange(addr, 0x7000, 0x7100))
	{
	    namco_06xx->write_data(data);
	}
	else if (addr == 0x7100)
	{
	    namco_06xx->write_control(data);
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    video_core->writeByte(addr, data);
	}
	else if (inRange(addr, 0x8800, 0x8C00))
	{
	    main_ram1.at(ram_addr) = data;

	    if (ram_addr >= 0x380)
	    {
		video_core->writeSprites(0, ram_addr, data);
	    }
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    main_ram2.at(ram_addr) = data;

	    if (ram_addr >= 0x380)
	    {
		video_core->writeSprites(1, ram_addr, data);
	    }
	}
	else if (inRange(addr, 0x9800, 0x9C00))
	{
	    main_ram3.at(ram_addr) = data;

	    if (ram_addr >= 0x380)
	    {
		video_core->writeSprites(2, ram_addr, data);
	    }
	}
	else if (inRange(addr, 0xA000, 0xA008))
	{
	    // TODO: Implement screen flipping and starfield
	    return;
	}
	else
	{
	    cout << "Writing value of " << hex << int(data) << " to Galaga address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    void GalagaCore::write_latch(int addr, uint8_t data)
    {
	addr &= 7;
	bool line = testbit(data, 0);

	switch (addr)
	{
	    case 0:
	    {
		main_irq_enabled = line;

		if (!line)
		{
		    main_proc->fire_interrupt8(0xFF, false);
		}
	    }
	    break;
	    case 1:
	    {
		aux_irq_enabled = line;

		if (!line)
		{
		    aux_proc->fire_interrupt8(0xFF, false);
		}
	    }
	    break;
	    case 2: sound_irq_enabled = !line; break;
	    case 3:
	    {
		if (!line)
		{
		    cout << "Asserting reset line..." << endl;
		    aux_cpu->suspend(SuspendReason::Reset);
		    sound_cpu->suspend(SuspendReason::Reset);
		    namco_51xx->set_reset_line(true);
		    // namco_54xx->set_reset_line(true);
		}
		else
		{
		    cout << "Clearing reset line..." << endl;

		    if (aux_cpu->is_suspended(SuspendReason::Reset))
		    {
			aux_cpu->reset();
			aux_cpu->resume(SuspendReason::Reset);
		    }

		    if (sound_cpu->is_suspended(SuspendReason::Reset))
		    {
			sound_cpu->reset();
			sound_cpu->resume(SuspendReason::Reset);
		    }

		    namco_51xx->set_reset_line(false);
		    // namco_54xx->set_reset_line(false);
		}
	    }
	    break;
	    default: break;
	}
    }

    void GalagaCore::chip_select_galaga(int addr, bool line)
    {
	addr &= 3;

	string line_str = (line) ? "high" : "low";

	switch (addr)
	{
	    case 0: namco_51xx->chip_select(line); break;
	    // case 3: namco_54xx->chip_select(line); break;
	    default: break;
	}
    }

    void GalagaCore::rw_galaga(int addr, bool line)
    {
	addr &= 3;

	switch (addr)
	{
	    case 0: namco_51xx->rw(line); break;
	    default: break;
	}
    }

    uint8_t GalagaCore::read_galaga(int addr)
    {
	uint8_t data = 0;
	addr &= 3;

	switch (addr)
	{
	    case 0: data = namco_51xx->read(); break;
	    default: break;
	}

	return data;
    }

    void GalagaCore::write_galaga(int addr, uint8_t data)
    {
	addr &= 3;

	switch (addr)
	{
	    case 0: namco_51xx->write(data); break;
	    // case 3: namco_54xx->write(data); break;
	    default: break;
	}
    }

    uint8_t GalagaCore::read_io_ports(int addr)
    {
	uint8_t data = 0;
	addr &= 3;

	switch (addr)
	{
	    case 0: data = (port0_val & 0xF); break;
	    case 1: data = (port0_val >> 4); break;
	    case 2: data = (port1_val & 0xF); break;
	    case 3: data = (port1_val >> 4); break;
	    default: data = 0; break;
	}

	return data;
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

    bool drivergalaga::hasdriverROMs()
    {
	return true;
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
	core->shutdown_core();
    }
  
    void drivergalaga::drvrun()
    {
	core->run_core();
    }

    void drivergalaga::keychanged(BerrnInput key, bool is_pressed)
    {
	core->key_changed(key, is_pressed);
    }
};