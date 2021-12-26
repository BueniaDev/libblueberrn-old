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

#include "galaga.h"
using namespace berrn;

namespace berrn
{
    GalagaInterface::GalagaInterface(GalagaCore &core) : parent_core(core)
    {
	bitmap = new BerrnBitmapRGB(224, 288);
	bitmap->clear();
    }

    GalagaInterface::~GalagaInterface()
    {

    }

    void GalagaInterface::init()
    {
	vram.fill(0);
	ram1.fill(0);
	ram2.fill(0);
	ram3.fill(0);
	tile_rom.resize(0x1000, 0);
	tile_ram.resize((256 * 8 * 8), 0);
	decode_images();
	dsw_a = 0xF7;
	dsw_b = 0x97;
    }

    void GalagaInterface::shutdown()
    {
	ram3.fill(0);
	ram2.fill(0);
	ram1.fill(0);
	vram.fill(0);
	bitmap->clear();
    }

    void GalagaInterface::decode_images()
    {
	for (int tile_num = 0; tile_num < 256; tile_num++)
	{
	    uint8_t *dst = &tile_ram[(tile_num * 64)];
	    const uint8_t *src = &tile_rom[(tile_num * 16)];
	    decode_strip((src + 0), dst, 0, 4, 8);
	    decode_strip((src + 8), dst, 0, 0, 8);
	}
    }

    // TODO: Implement native tile decoding in the core driver API
    void GalagaInterface::decode_strip(const uint8_t *src, uint8_t *dst, int bx, int by, int width)
    {
	int base_index = (bx + (by * width));

	for (int xpos = 0; xpos < 8; xpos++)
	{
	    uint8_t strip = src[xpos];

	    for (int ypos = 0; ypos < 4; ypos++)
	    {
		int index = ((3 - ypos) * width + (7 - xpos));
		uint32_t addr = (base_index + index);
		dst[addr] = changebit(dst[addr], 0, testbit(strip, ypos));
		dst[addr] = changebit(dst[addr], 1, testbit(strip, (4 + ypos)));
	    }
	}
    }

    array<uint8_t, 4> GalagaInterface::get_palette(int pal_num)
    {
	array<uint8_t, 4> palette;
	int pal_offs = ((pal_num & 0x3F) * 4);

	for (int i = 0; i < 4; i++)
	{
	    palette[i] = pal_rom.at((pal_offs + i));
	}

	return palette;
    }

    void GalagaInterface::set_pixel(int xpos, int ypos, uint8_t color_num)
    {
	if (!inRange(xpos, 0, 224) || !inRange(ypos, 0, 288))
	{
	    return;
	}

	uint8_t color = color_rom[color_num];

	bool red0 = testbit(color, 0);
	bool red1 = testbit(color, 1);
	bool red2 = testbit(color, 2);

	bool green0 = testbit(color, 3);
	bool green1 = testbit(color, 4);
	bool green2 = testbit(color, 5);

	bool blue0 = testbit(color, 6);
	bool blue1 = testbit(color, 7);

	int red = ((red0 * 0x21) + (red1 * 0x47) + (red2 * 0x97));
	int green = ((green0 * 0x21) + (green1 * 0x47) + (green2 * 0x97));
	int blue = ((blue0 * 0x51) + (blue1 * 0xAE));

	berrnRGBA pixel_color = fromRGB(red, green, blue);
	bitmap->setPixel(xpos, ypos, pixel_color);
    }

    void GalagaInterface::draw_tile(uint8_t tile_num, array<uint8_t, 4> palette, int xcoord, int ycoord)
    {
	int base_x = ((xcoord - 2) * 8);
	int base_y = (ycoord * 8);

	if (!inRange(base_x, 0, 224))
	{
	    return;
	}

	for (int pixel = 0; pixel < 64; pixel++)
	{
	    int py = (pixel / 8);
	    int px = (pixel % 8);

	    uint8_t color_num = tile_ram[((tile_num * 64) + pixel)];
	    int xpos = (base_x + px);
	    int ypos = (base_y + py);
	    set_pixel(xpos, ypos, palette[color_num]);
	}
    }

    uint8_t GalagaInterface::readByte(uint16_t addr)
    {
	auto namco06 = parent_core.get_namco_06xx();

	uint8_t data = 0;
	if (!isValidAddr(addr)) // This shouldn't happen
	{
	    cout << "Reading byte from invalid address of " << hex << int(addr) << endl;
	    exit(0);
	    data = 0;
	}
	else if (inRange(addr, 0x6800, 0x6808))
	{
	    int bit_index = (addr & 7);
	    bool b1 = testbit(dsw_a, bit_index);
	    bool b0 = testbit(dsw_b, bit_index);
	    data = ((b1 << 1) | b0);
	}
	else if (addr == 0x7100)
	{
	    if (namco06 != NULL)
	    {
		data = namco06->readControl();
	    }
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    data = vram[(addr - 0x8000)];
	}
	else if (inRange(addr, 0x8800, 0x8C00))
	{
	    data = ram1[(addr - 0x8800)];
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    data = ram2[(addr - 0x9000)];
	}
	else if (inRange(addr, 0x9800, 0x9C00))
	{
	    data = ram3[(addr - 0x9800)];
	}
	else
	{
	    cout << "Reading byte from interface address of " << hex << int(addr) << endl;
	    exit(0);
	    data = 0;
	}
	return data;
    }

    void GalagaInterface::writeByte(uint16_t addr, uint8_t data)
    {
	auto namco06 = parent_core.get_namco_06xx();

	if (!isValidAddr(addr)) // This shouldn't happen
	{
	    cout << "Writing byte of " << hex << int(data) << " to invalid address of " << hex << int(addr) << endl;
	    exit(0);
	}
	else if (inRange(addr, 0x6820, 0x6828))
	{
	    parent_core.writeLatch((addr & 7), data);
	}
	else if (addr == 0x6830)
	{
	    // Watchdog timer (not implemented)
	    return;
	}
	else if (inRange(addr, 0x7000, 0x7100))
	{
	    if (namco06 != NULL)
	    {
		namco06->writeData(data);
	    }
	}
	else if (addr == 0x7100)
	{
	    if (namco06 != NULL)
	    {
		namco06->writeControl(data);
	    }
	}
	else if (inRange(addr, 0x8000, 0x8800))
	{
	    vram[(addr - 0x8000)] = data;
	}
	else if (inRange(addr, 0x8800, 0x8C00))
	{
	    ram1[(addr - 0x8800)] = data;
	}
	else if (inRange(addr, 0x9000, 0x9400))
	{
	    ram2[(addr - 0x9000)] = data;
	}
	else if (inRange(addr, 0x9800, 0x9C00))
	{
	    ram3[(addr - 0x9800)] = data;
	}
	else if (addr == 0xA007)
	{
	    return;
	}
	else
	{
	    cout << "Writing byte of " << hex << int(data) << " to interface address of " << hex << int(addr) << endl;
	    exit(0);
	}
    }

    bool GalagaInterface::isValidAddr(uint16_t addr)
    {
	return inRange(addr, 0x6800, 0xA008);
    }

    void GalagaInterface::updatePixels()
    {
	for (int addr = 0; addr < 64; addr++)
	{
	    int ypos = (34 + (addr / 32));
	    int xpos = (31 - (addr % 32));

	    uint8_t tile_num = vram[addr];
	    uint8_t pal_num = vram[(0x400 + addr)];

	    auto palette = get_palette(pal_num);
	    draw_tile(tile_num, palette, xpos, ypos);
	}

	for (int addr = 0; addr < 0x380; addr++)
	{
	    int ypos = (2 + (addr % 32));
	    int xpos = (29 - (addr / 32));

	    uint8_t tile_num = vram[(0x40 + addr)];
	    uint8_t pal_num = vram[(0x440 + addr)];

	    auto palette = get_palette(pal_num);
	    draw_tile(tile_num, palette, xpos, ypos);
	}

	for (int addr = 0; addr < 64; addr++)
	{
	    int ypos = (addr / 32);
	    int xpos = (31 - (addr % 32));

	    uint8_t tile_num = vram[(0x3C0 + addr)];
	    uint8_t pal_num = vram[(0x7C0 + addr)];

	    auto palette = get_palette(pal_num);
	    draw_tile(tile_num, palette, xpos, ypos);
	}
    }

    GalagaCPU::GalagaCPU(GalagaInterface *cb) : inter(cb)
    {

    }

    GalagaCPU::~GalagaCPU()
    {

    }

    void GalagaCPU::init()
    {
	core_rom.resize(0x4000, 0);
	inter->init();
    }

    void GalagaCPU::shutdown()
    {
	inter->shutdown();
	core_rom.clear();
    }

    uint8_t GalagaCPU::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void GalagaCPU::writeCPU8(uint16_t addr, uint8_t data)
    {
	if (addr < 0x4000)
	{
	    return;
	}
	else
	{
	    if (inter == NULL)
	    {
		cout << "Error: interface is NULL" << endl;
		exit(1);
	    }
	    else if (!inter->isValidAddr(addr))
	    {
		BerrnInterface::writeCPU8(addr, data);
	    }
	    else
	    {
		inter->writeByte(addr, data);
	    }
	}
    }

    uint8_t GalagaCPU::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t GalagaCPU::readByte(uint16_t addr)
    {
	uint8_t temp = 0;
	if (addr < 0x4000)
	{
	    temp = core_rom.at(addr);
	}
	else
	{
	    if (inter == NULL)
	    {
		cout << "Error: interface is NULL" << endl;
		exit(1);
		temp = 0;
	    }
	    else if (!inter->isValidAddr(addr))
	    {
		cout << "Reading from address of " << hex << int(addr) << endl;
		exit(0);
		temp = 0;
	    }
	    else
	    {
		temp = inter->readByte(addr);
	    }
	}

	return temp;
    }

    GalagaCore::GalagaCore(berrndriver &drv) : driver(drv)
    {
	scheduler.set_interleave(100);
	shared_inter = new GalagaInterface(*this);
	main_inter = new GalagaCPU(shared_inter);
	main_proc = new BerrnZ80Processor(3072000, *main_inter);
	main_cpu = new BerrnCPU(scheduler, *main_proc);

	aux_inter = new GalagaCPU(shared_inter);
	aux_proc = new BerrnZ80Processor(3072000, *aux_inter);
	aux_cpu = new BerrnCPU(scheduler, *aux_proc);

	sound_inter = new GalagaCPU(shared_inter);
	sound_proc = new BerrnZ80Processor(3072000, *sound_inter);
	sound_cpu = new BerrnCPU(scheduler, *sound_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t) {
	    shared_inter->updatePixels();
	});

	interrupt_timer = new BerrnTimer("IRQ1", scheduler, [&](int64_t, int64_t) {
	    if (aux_irq_enabled && !aux_cpu->is_suspended(SuspendReason::Reset))
	    {
		aux_proc->fire_interrupt8(0xFF);
	    }
	});

	namco_06xx = new namco06xx(*main_cpu);

	// Namco 51xx MCU clock rate is derived from the main clock frequency
	namco_51xx = new namco51xx(drv, scheduler, 3072000);
    }

    GalagaCore::~GalagaCore()
    {

    }

    void GalagaCore::init()
    {
	scheduler.reset();
	scheduler.add_device(main_cpu);
	scheduler.add_device(aux_cpu);
	scheduler.add_device(sound_cpu);
	main_proc->init();
	aux_proc->init();
	sound_proc->init();
	main_inter->init();
	aux_inter->init();
	sound_inter->init();
	shared_inter->init();
	namco_51xx->init();

	interrupt_timer->start(time_in_hz(60), true);
	vblank_timer->start(time_in_hz(60), true);
    }

    void GalagaCore::shutdown()
    {
	vblank_timer->stop();
	interrupt_timer->stop();
	namco_51xx->shutdown();
	shared_inter->shutdown();
	sound_inter->shutdown();
	aux_inter->shutdown();
	main_inter->shutdown();
	sound_proc->shutdown();
	aux_proc->shutdown();
	main_proc->shutdown();
	scheduler.remove_device(sound_cpu);
	scheduler.remove_device(aux_cpu);
	scheduler.remove_device(main_cpu);
    }

    void GalagaCore::writeLatch(int offset, uint8_t data)
    {
	switch (offset)
	{
	    case 0x1:
	    {
		aux_irq_enabled = testbit(data, 0);
	    }
	    break;
	    case 0x2:
	    {
		sound_irq_enabled = !testbit(data, 0);
	    }
	    break;
	    case 0x3:
	    {
		if (!testbit(data, 0))
		{
		    cout << "Asserting reset line..." << endl;
		    aux_cpu->suspend(SuspendReason::Reset);
		    sound_cpu->suspend(SuspendReason::Reset);
		    namco_51xx->set_reset_line(true);
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
		}
	    }
	    break;
	    default:
	    {
		cout << "Writing value of " << hex << int(data) << " to latch of " << dec << int(offset) << endl;
		exit(0);
	    }
	    break;
	}
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

    void GalagaCore::keychanged(BerrnInput key, bool is_pressed)
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
	berrn_rom_region("color", 0x0020)
	    berrn_rom_load("prom-5.5n", 0x0000, 0x0020)
	berrn_rom_region("pal", 0x0100)
	    berrn_rom_load("prom-4.2n", 0x0000, 0x0100)
    berrn_rom_end

    drivergalaga::drivergalaga()
    {
	galaga_core = new GalagaCore(*this);
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

    bool drivergalaga::loadROMs()
    {
        if (!loadROM(berrn_rom_name(galaga)))
	{
	    return false;
	}

	galaga_core->set_rom(CPUType::Main, get_rom_region("maincpu"));
	galaga_core->set_rom(CPUType::Aux, get_rom_region("auxcpu"));
	galaga_core->set_rom(CPUType::Sound, get_rom_region("soundcpu"));
	galaga_core->set_tile_rom(get_rom_region("gfx1"));
	galaga_core->set_pal_rom(get_rom_region("pal"));
	galaga_core->set_color_rom(get_rom_region("color"));
	return true;
    }

    bool drivergalaga::drvinit()
    {
	if (!loadROMs())
	{
	    return false;
	}

	galaga_core->init();
	resize(224, 288, 2);
	return true;
    }

    void drivergalaga::drvshutdown()
    {
	galaga_core->shutdown();
    }
  
    void drivergalaga::drvrun()
    {
	galaga_core->run_core();
	setScreen(galaga_core->getBitmap());
    }

    void drivergalaga::keychanged(BerrnInput key, bool is_pressed)
    {
	galaga_core->keychanged(key, is_pressed);
    }
};