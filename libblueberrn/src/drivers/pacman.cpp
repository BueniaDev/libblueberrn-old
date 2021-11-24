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

#include "pacman.h"
using namespace berrn;

namespace berrn
{
    PacmanInterface::PacmanInterface()
    {

    }

    PacmanInterface::~PacmanInterface()
    {

    }

    void PacmanInterface::init()
    {
	in0_val = 0xFF;
	in1_val = 0xFF;
	gamerom.resize(0x4000, 0);
	vram.fill(0);
	cram.fill(0);
	mainram.fill(0);
	framebuffer.fill(black());
	color_rom.resize(0x20, 0);
	pal_rom.resize(0x100, 0);
	tile_rom.resize(0x1000, 0);
	sprite_rom.resize(0x1000, 0);
	tile_ram.resize((256 * 8 * 8), 0);
	sprite_ram.resize((64 * 16 * 16), 0);
	sound_rom.resize(512, 0);
	decode_images();
	pac_sound.init(sound_rom);
    }

    void PacmanInterface::decode_images()
    {
	for (int tile_num = 0; tile_num < 256; tile_num++)
	{
	    uint8_t *dst = &tile_ram[(tile_num * 64)];
	    const uint8_t *src = &tile_rom[(tile_num * 16)];
	    decode_strip((src + 0), dst, 0, 4, 8);
	    decode_strip((src + 8), dst, 0, 0, 8);
	}

	for (int sprite_num = 0; sprite_num < 64; sprite_num++)
	{
	    uint8_t *dst = &sprite_ram[(sprite_num * 16 * 16)];
	    const uint8_t *src = &sprite_rom[(sprite_num * 64)];

	    decode_strip((src + 0), dst, 8, 12, 16);
	    decode_strip((src + 8), dst, 8, 0, 16);
	    decode_strip((src + 16), dst, 8, 4, 16);
	    decode_strip((src + 24), dst, 8, 8, 16);

	    decode_strip((src + 32), dst, 0, 12, 16);
	    decode_strip((src + 40), dst, 0, 0, 16);
	    decode_strip((src + 48), dst, 0, 4, 16);
	    decode_strip((src + 56), dst, 0, 8, 16);
	}
    }

    // TODO: Implement native tile decoding in the core driver API
    void PacmanInterface::decode_strip(const uint8_t *src, uint8_t *dst, int bx, int by, int width)
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

    void PacmanInterface::shutdown()
    {
	pac_sound.shutdown();
	sound_rom.clear();
	sprite_ram.clear();
	sprite_rom.clear();
	tile_ram.clear();
	tile_rom.clear();
	pal_rom.clear();
	color_rom.clear();
	gamerom.clear();
    }

    uint8_t PacmanInterface::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void PacmanInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FFF;

	if (addr < 0x4000)
	{
	    return;
	}
	else if (addr < 0x4400)
	{
	    vram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x4800)
	{
	    cram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x4C00)
	{
	    return;
	}
	else if (addr < 0x4FF0)
	{
	    mainram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x5000)
	{
	    oam[(addr & 0xF)] = data;
	}
	else if (addr < 0x5040)
	{
	    writeIO((addr & 7), data);
	}
	else if (addr < 0x5060)
	{
	    pac_sound.writereg((addr - 0x5040), data);
	}
	else if (addr < 0x5070)
	{
	    sprite_pos[(addr & 0xF)] = data;
	}
	else if (addr < 0x50C0)
	{
	    return;
	}
	else
	{
	    // Watchdog timer (currently unimplemented)
	    return;
	}
    }

    uint8_t PacmanInterface::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t PacmanInterface::readByte(uint16_t addr)
    {
	addr &= 0x7FFF;

	uint8_t data = 0;

	if (addr < 0x4000)
	{
	    data = gamerom[addr];
	}
	else if (addr < 0x4400)
	{
	    data = vram[(addr & 0x3FF)];
	}
	else if (addr < 0x4800)
	{
	    data = cram[(addr & 0x3FF)];
	}
	else if (addr < 0x4C00)
	{
	    data = 0x00;
	}
	else if (addr < 0x4FF0)
	{
	    data = mainram[(addr & 0x3FF)];
	}
	else if (addr < 0x5000)
	{
	    data = oam[(addr & 0xF)];
	}
	else if (addr < 0x5040)
	{
	    data = in0_val;
	}
	else if (addr < 0x5080)
	{
	    data = in1_val;
	}
	else if (addr < 0x50C0)
	{
	    // Reading IN2
	    data = 0xC9;
	}
	else
	{
	    cout << "Reading from address of " << hex << int(addr) << endl;
	    exit(0);
	}

	return data;
    }

    void PacmanInterface::writeIO(int addr, uint8_t data)
    {
	switch (addr)
	{
	    case 0: vblank_enable = testbit(data, 0); break;
	    case 1: sound_enable = testbit(data, 0); break;
	    case 3: flip_screen = testbit(data, 0); break;
	}
    }

    void PacmanInterface::portOut(uint16_t port, uint8_t data)
    {
	if ((port & 0xFF) == 0)
	{
	    int_vector = data;
	}
	else
	{
	    BerrnInterface::portOut(port, data);
	}
    }

    void PacmanInterface::set_pixel(int xpos, int ypos, uint8_t color_num)
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
	int screen_index = (xpos + (ypos * 224));
	framebuffer.at(screen_index) = pixel_color;
    }

    void PacmanInterface::draw_tile(uint8_t tile_num, array<uint8_t, 4> palette, int xcoord, int ycoord)
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

    void PacmanInterface::draw_sprite(uint8_t sprite_num, array<uint8_t, 4> palette, int xcoord, int ycoord, bool flipx, bool flipy)
    {
	if (!inRange(xcoord, -16, 224))
	{
	    return;
	}

	int base_x = xcoord;
	int base_y = ycoord;

	for (int pixel = 0; pixel < (16 * 16); pixel++)
	{
	    int px = (pixel % 16);
	    int py = (pixel / 16);

	    if (flipx)
	    {
		px = (15 - px);
	    }

	    if (flipy)
	    {
		py = (15 - py);
	    }

	    int xpos = (base_x + px);
	    int ypos = (base_y + py);

	    if (!inRange(xpos, 0, 224))
	    {
		continue;
	    }

	    uint8_t color_num = sprite_ram[((sprite_num * 256) + pixel)];

	    if (palette[color_num] == 0)
	    {
		continue;
	    }

	    set_pixel(xpos, ypos, palette[color_num]);
	}
    }

    array<uint8_t, 4> PacmanInterface::get_palette(int pal_num)
    {
	array<uint8_t, 4> palette;
	int pal_offs = ((pal_num & 0x3F) * 4);

	for (int i = 0; i < 4; i++)
	{
	    palette[i] = pal_rom.at((pal_offs + i));
	}

	return palette;
    }

    void PacmanInterface::updatePixels()
    {
	for (int addr = 0; addr < 64; addr++)
	{
	    int ypos = (34 + (addr / 32));
	    int xpos = (31 - (addr % 32));

	    uint8_t tile_num = vram[addr];
	    uint8_t pal_num = cram[addr];

	    auto palette = get_palette(pal_num);
	    draw_tile(tile_num, palette, xpos, ypos);
	}

	for (int addr = 0; addr < 0x380; addr++)
	{
	    int ypos = (2 + (addr % 32));
	    int xpos = (29 - (addr / 32));

	    uint8_t tile_num = vram[(64 + addr)];
	    uint8_t pal_num = cram[(64 + addr)];

	    auto palette = get_palette(pal_num);
	    draw_tile(tile_num, palette, xpos, ypos);
	}

	for (int addr = 0; addr < 64; addr++)
	{
	    int ypos = (addr / 32);
	    int xpos = (31 - (addr % 32));

	    uint8_t tile_num = vram[(0x3C0 + addr)];
	    uint8_t pal_num = cram[(0x3C0 + addr)];

	    auto palette = get_palette(pal_num);
	    draw_tile(tile_num, palette, xpos, ypos);
	}

	for (int sprite = 7; sprite >= 0; sprite--)
	{
	    int16_t xpos = (224 - sprite_pos[(sprite * 2)] + 15);
	    int16_t ypos = (288 - sprite_pos[((sprite * 2) + 1)] - 16);

	    uint8_t sprite_info = oam[(sprite * 2)];
	    uint8_t pal_num = oam[((sprite * 2) + 1)];

	    bool xflip = testbit(sprite_info, 1);
	    bool yflip = testbit(sprite_info, 0);
	    uint8_t sprite_num = (sprite_info >> 2);

	    auto palette = get_palette(pal_num);
	    draw_sprite(sprite_num, palette, xpos, ypos, xflip, yflip);
	}
    }

    void PacmanInterface::coin(bool is_pressed)
    {
	in0_val = changebit(in0_val, 5, !is_pressed);
    }

    void PacmanInterface::p1up(bool is_pressed)
    {
	in0_val = changebit(in0_val, 0, !is_pressed);
    }

    void PacmanInterface::p1left(bool is_pressed)
    {
	in0_val = changebit(in0_val, 1, !is_pressed);
    }

    void PacmanInterface::p1right(bool is_pressed)
    {
	in0_val = changebit(in0_val, 2, !is_pressed);
    }

    void PacmanInterface::p1down(bool is_pressed)
    {
	in0_val = changebit(in0_val, 3, !is_pressed);
    }

    void PacmanInterface::p1start(bool is_pressed)
    {
	in1_val = changebit(in1_val, 5, !is_pressed);
    }

    driverpacman::driverpacman()
    {
	pacman_proc = new BerrnZ80Processor(3072000, inter);
	pacman_cpu = new BerrnCPU(scheduler, *pacman_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t param, int64_t) {
	    if (param == 1)
	    {
		vblank_timer->start(time_in_hz(60), true);
	    }

	    inter.updatePixels();
	});

	interrupt_timer = new BerrnTimer("Interrupt", scheduler, [&](int64_t, int64_t) {
	    this->interrupt_handler();
	});

	sound_timer = new BerrnTimer("Sound", scheduler, [&](int64_t, int64_t) {
	    if (inter.is_sound_enabled())
	    {
		this->sound_clock();
	    }
	});
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

    void driverpacman::loadROMs()
    {
	loadROM("pacman.6e", 0x0000, 0x1000, inter.get_gamerom());
	loadROM("pacman.6f", 0x1000, 0x1000, inter.get_gamerom());
	loadROM("pacman.6h", 0x2000, 0x1000, inter.get_gamerom());
	loadROM("pacman.6j", 0x3000, 0x1000, inter.get_gamerom());
	loadROM("82s123.7f", 0x0000, 0x0020, inter.get_color_rom());
	loadROM("82s126.4a", 0x0000, 0x0100, inter.get_pal_rom());
	loadROM("pacman.5e", 0x0000, 0x1000, inter.get_tile_rom());
	loadROM("pacman.5f", 0x0000, 0x1000, inter.get_sprite_rom());
	loadROM("82s126.1m", 0x0000, 0x0100, inter.get_sound_rom());
	loadROM("82s126.3m", 0x0100, 0x0100, inter.get_sound_rom());
    }

    void driverpacman::interrupt_handler()
    {
	if (inter.is_vblank_enabled())
	{
	    pacman_proc->fire_interrupt8(inter.get_int_vec());
	}
    }

    void driverpacman::sound_clock()
    {
	while (out_step > out_time)
	{
	    inter.clock_wsg3();
	    out_time += in_step;
	}

	out_time -= out_step;
	auto sample = inter.get_wsg3_sample();
	mixSample(sample);
	outputAudio();
    }

    bool driverpacman::drvinit()
    {
	int64_t frame_time = time_in_hz(60);
	out_step = inter.get_wsg3_sample_rate();
	in_step = getSampleRate();
	out_time = 0.0f;
	loadROMs();
	scheduler.reset();
	scheduler.add_device(pacman_cpu);
	interrupt_timer->start(frame_time, true);
	vblank_timer->start((frame_time - 2500), false, 1);
	sound_timer->start(time_in_hz(getSampleRate()), true);
	pacman_proc->init();
	inter.init();
	resize(224, 288, 2);
	return isallfilesloaded();
    }

    void driverpacman::drvshutdown()
    {
	sound_timer->stop();
	vblank_timer->stop();
	inter.shutdown();
	pacman_proc->shutdown();
	scheduler.remove_timer(sound_timer);
	scheduler.remove_timer(vblank_timer);
	scheduler.remove_timer(interrupt_timer);
	scheduler.remove_device(pacman_cpu);
    }
  
    void driverpacman::drvrun()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = time_in_hz(60);

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}

	filltexrect(0, 0, 224, 288, inter.get_framebuffer());
    }

    void driverpacman::keychanged(BerrnInput key, bool is_pressed)
    {
	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		inter.coin(is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnStartP1:
	    {
		inter.p1start(is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnUpP1:
	    {
		inter.p1up(is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnDownP1:
	    {
		inter.p1down(is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnLeftP1:
	    {
		inter.p1left(is_pressed);
	    }
	    break;
	    case BerrnInput::BerrnRightP1:
	    {
		inter.p1right(is_pressed);
	    }
	    break;
	    default: break;
	}
    }
};