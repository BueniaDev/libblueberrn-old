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

#include "galaxian.h"
using namespace berrn;

namespace berrn
{
    GalaxianInterface::GalaxianInterface()
    {

    }

    GalaxianInterface::~GalaxianInterface()
    {

    }

    void GalaxianInterface::init()
    {
	gameram.fill(0);
	vram.fill(0);
	special_ram.fill(0);
	gamerom.resize(0x4000, 0);
	char_data.resize((256 * 8 * 8 * 2), 0);
	sprite_data.resize((64 * 16 * 16 * 2), 0);
	decode_images();
	framebuffer.fill(black());
	init_starfield();
    }

    void GalaxianInterface::init_starfield()
    {
	// Initial setup derived from the MAME driver in galaxian.c
	int stars = 0;
	uint32_t generator = 0;
	for (int ypos = 0; ypos < 512; ypos++)
	{
	    for (int xpos = 0; xpos < 256; xpos++)
	    {
		bool gen_bit = (!testbit(generator, 16) != testbit(generator, 4));
		generator = ((generator << 1) | gen_bit);

		if (!testbit(generator, 16) && (uint8_t(generator) == 0xFF))
		{
		    int color = ((~(generator >> 8)) & 0x3F);

		    if (color != 0)
		    {
			if (stars < 2520)
			{
			    starfield[stars].xpos = xpos;
			    starfield[stars].ypos = ypos;
			    starfield[stars].color = color;

			    stars += 1;
			}
		    }
		}
	    }
	}

	starfield_scroll_pos = 0;
    }

    void GalaxianInterface::shutdown()
    {
	sprite_data.clear();
	char_data.clear();
	gamerom.clear();
    }

    uint8_t GalaxianInterface::readCPU8(uint16_t addr)
    {
	return readByte(addr);
    }

    void GalaxianInterface::writeCPU8(uint16_t addr, uint8_t data)
    {
	addr &= 0x7FFF;
	if (addr < 0x4000)
	{
	    return;
	}
	else if (addr < 0x5000)
	{
	    gameram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x5800)
	{
	    vram[(addr & 0x3FF)] = data;
	}
	else if (addr < 0x6000)
	{
	    special_ram[(addr & 0xFF)] = data;
	}
	else if (addr < 0x6800)
	{
	    int sound_addr = (addr & 7);

	    if (sound_addr >= 4)
	    {
		cout << "Writing value of " << hex << int(data) << " to soundboard lower address of " << hex << int(sound_addr) << endl;
	    }

	    return;
	}
	else if (addr < 0x7000)
	{
	    int sound_addr = (addr & 7);
	    cout << "Writing value of " << hex << int(data) << " to soundboard upper address of " << hex << int(sound_addr) << endl;
	    return;
	}
	else
	{
	    writeIOupper(addr, data);
	}
    }

    uint8_t GalaxianInterface::readOp8(uint16_t addr)
    {
	return readByte(addr);
    }

    uint8_t GalaxianInterface::readByte(uint16_t addr)
    {
	addr &= 0x7FFF;
	uint8_t data = 0;

	if (addr < 0x4000)
	{
	    data = gamerom[addr];
	}
	else if (addr < 0x5000)
	{
	    data = gameram[(addr & 0x3FF)];
	}
	else if (addr < 0x5800)
	{
	    data = vram[(addr & 0x3FF)];
	}
	else if (addr < 0x6000)
	{
	    data = special_ram[(addr & 0xFF)];
	}
	else if (addr < 0x6800)
	{
	    data = port0_val;
	}
	else if (addr < 0x7000)
	{
	    // TODO: IN1 reads
	    data = 0x00;
	}
	else if (addr < 0x7800)
	{
	    // TODO: IN2 reads
	    data = 0x00;
	}
	else
	{
	    // Watchdog timer (unimplemented)
	    data = 0xFF;
	}

	return data;
    }

    void GalaxianInterface::writeIOupper(uint16_t addr, uint8_t data)
    {
	if (addr < 0x7800)
	{
	    switch ((addr & 0x7))
	    {
		case 0x01: is_irq_enabled = testbit(data, 0); break;
		case 0x04: is_stars_enabled = testbit(data, 0); break;
		case 0x06:
		{
		    if (testbit(data, 0))
		    {
			cout << "Screen is flipped horizontally" << endl;
		    }
		    else
		    {
			cout << "Screen is normal at x-axis" << endl;
		    }
		}
		break;
		case 0x07:
		{
		    if (testbit(data, 0))
		    {
			cout << "Screen is flipped vertically" << endl;
		    }
		    else
		    {
			cout << "Screen is normal at y-axis" << endl;
		    }
		}
		break;
	    }
	}
	else
	{
	    cout << "Writing value of " << hex << int(data) << " to soundboard pitch register" << endl;
	}
    }

    array<uint8_t, 4> GalaxianInterface::get_palette(int color)
    {
	array<uint8_t, 4> palette_colors;
	int color_offs = ((color & 0x7) * 4);

	for (int i = 0; i < 4; i++)
	{
	    palette_colors[i] = palrom.at((color_offs + i));
	}

	return palette_colors;
    }

    void GalaxianInterface::decode_images()
    {
	for (int i = 0; i < 256; i++)
	{
	    auto tile_src = &tilerom[(i * 8)];
	    auto tile_dst = &char_data[(i * 64)];
	    decode_tile(tile_src, tile_dst);
	}

	for (int i = 0; i < 64; i++)
	{
	    auto obj_src = &tilerom[(i * 32)];
	    auto obj_dst = &sprite_data[(i * 256)];
	    decode_sprite(obj_src, obj_dst);
	}
    }

    void GalaxianInterface::coin(bool is_pressed)
    {
	port0_val = changebit(port0_val, 0, is_pressed);
    }

    void GalaxianInterface::star_scroll_callback()
    {
	if (is_stars_enabled)
	{
	    starfield_scroll_pos += 1;
	}
	else
	{
	    starfield_scroll_pos = 0;
	}
    }

    void GalaxianInterface::set_raw_pixel(int xpos, int ypos, berrnRGBA color)
    {
	if (!inRange(xpos, 0, width) || !inRange(ypos, 0, height))
	{
	    return;
	}

	int screen_index = (xpos + (ypos * width));
	framebuffer.at(screen_index) = color;
    }

    // TODO: Implement native tile decoding in the core driver API
    void GalaxianInterface::decode_char(const uint8_t *src, uint8_t *dst, int ox, int oy, int width)
    {
	for (int ypos = 0; ypos < 8; ypos++)
	{
	    for (int xpos = 0; xpos < 8; xpos++)
	    {
		uint8_t color = 0;
		uint32_t plane_offs = 0;

		for (int plane = 1; plane >= 0; plane--)
		{
		    if (testbit(src[plane_offs + ypos], xpos))
		    {
			color = setbit(color, plane);
		    }

		    plane_offs += 2048;
		}

		int posx = (ox + (7 - ypos));
		int posy = (oy + (7 - xpos));
		int pixel_index = ((posy * width) + posx);
		dst[pixel_index] = color;
	    }
	}
    }

    void GalaxianInterface::decode_tile(const uint8_t *src, uint8_t *dst)
    {
	decode_char(src, dst, 0, 0, 8);
    }

    void GalaxianInterface::decode_sprite(const uint8_t *src, uint8_t *dst)
    {
	decode_char(src, dst, 8, 0, 16);
	decode_char((src + 8), dst, 8, 8, 16);
	decode_char((src + 16), dst, 0, 0, 16);
	decode_char((src + 24), dst, 0, 8, 16);
    }

    void GalaxianInterface::set_pixel(int xpos, int ypos, uint8_t color)
    {
	if (color == 0)
	{
	    return;
	}

	bool red0 = testbit(color, 0);
	bool red1 = testbit(color, 1);
	bool red2 = testbit(color, 2);

	bool green0 = testbit(color, 3);
	bool green1 = testbit(color, 4);
	bool green2 = testbit(color, 5);

	bool blue0 = testbit(color, 6);
	bool blue1 = testbit(color, 7);

	int red = ((red0 * 0x1D) + (red1 * 0x3E) + (red2 * 0x85));
	int green = ((green0 * 0x1D) + (green1 * 0x3E) + (green2 * 0x85));
	int blue = ((blue0 * 0x47) + (blue1 * 0x99));

	berrnRGBA pixel_color = fromRGB(red, green, blue);
	set_raw_pixel(xpos, ypos, pixel_color);
    }

    void GalaxianInterface::draw_star_pixel(int xpos, int ypos, int color)
    {
	color &= 0x3F;

	array<uint8_t, 4> starmap = {0, 194, 214, 255};

	int red = starmap.at((color & 0x3));
	int green = starmap.at(((color >> 2) & 0x3));
	int blue = starmap.at(((color >> 4) & 0x3));

	berrnRGBA pixel_color = fromRGB(red, green, blue);
	set_raw_pixel(xpos, ypos, pixel_color);
    }

    void GalaxianInterface::draw_tile(uint8_t tile_num, int xpos, int ypos, array<uint8_t, 4> palette)
    {
	if (!inRange(xpos, 0, width) || !inRange(ypos, 0, height))
	{
	    return;
	}

	for (int index = 0; index < 64; index++)
	{
	    int py = (index / 8);
	    int px = (index % 8);

	    uint8_t tile_color = char_data.at((tile_num * 64) + index);
	    int posy = (ypos + py);
	    int posx = (xpos + px);
	    uint8_t color = palette.at(tile_color);

	    set_pixel(posx, posy, color);
	}
    }

    void GalaxianInterface::draw_sprite(uint8_t sprite_num, int xpos, int ypos, bool flip_x, bool flip_y, array<uint8_t, 4> palette)
    {
	if (!inRange(xpos, 0, width) || !inRange(ypos, 0, height))
	{
	    return;
	}

	for (int index = 0; index < 256; index++)
	{
	    int py = (index / 16);
	    int px = (index % 16);

	    uint8_t sprite_color = sprite_data.at((sprite_num * 256) + index);

	    if (flip_x)
	    {
		px = (15 - px);
	    }

	    if (flip_y)
	    {
		py = (15 - py);
	    }

	    int posx = (xpos + px);
	    int posy = (ypos + py);

	    if (!inRange(posx, 0, width))
	    {
		continue;
	    }

	    uint8_t color = palette.at(sprite_color);
	    set_pixel(posx, posy, color);
	}
    }

    void GalaxianInterface::draw_starfield()
    {
	for (int i = 0; i < 2520; i++)
	{
	    int ypos = starfield[i].ypos + starfield_scroll_pos;
	    int xpos = ((starfield[i].xpos + (ypos >> 9)) & 0xFF);

	    ypos = ((ypos & 0x1FF) / 2);

	    if (testbit(xpos, 0) != testbit(ypos, 3))
	    {
		draw_star_pixel((xpos - 16), ypos, starfield[i].color);
	    }
	}
    }

    void GalaxianInterface::updatePixels()
    {
	// Render the background (using black color)...
	framebuffer.fill(black());

	// ...and the starfield
	if (is_stars_enabled)
	{
	    draw_starfield();
	}

	// ...followed by the tiles...
	for (int ypos = 0; ypos < 32; ypos++)
	{
	    uint8_t scroll = special_ram.at((ypos * 2));
	    int color = (special_ram.at((ypos * 2) + 1) & 7);

	    for (int xpos = 0; xpos < 32; xpos++)
	    {
		uint32_t offset = ((32 * (31 - xpos)) + ypos);

		int cx = ((((xpos * 8) + scroll) & 0xFF) - 16);
		int cy = (ypos * 8);

		if (inRange(cx, -7, 224))
		{
		    uint8_t char_code = vram.at(offset);
		    auto palette = get_palette(color);
		    draw_tile(char_code, cx, cy, palette);
		}
	    }
	}

	// ...then the bullets...
	for (int bullet = 0; bullet < 8; bullet++)
	{
	    uint32_t bullet_offs = (0x60 + (bullet * 4));
	    int xpos = (special_ram.at((bullet_offs + 1)) - 16);
	    int ypos = (255 - special_ram.at((bullet_offs + 3)));

	    berrnRGBA bullet_color = (bullet == 7) ? yellow() : white();

	    for (int index = 0; index < 4; index++)
	    {
		ypos -= 1;

		if (ypos >= 0)
		{
		    set_raw_pixel(xpos, ypos, bullet_color);
		}
	    }
	}

	// ...and lastly, the sprites
	for (int sprite = 7; sprite >= 0; sprite--)
	{
	    uint32_t sprite_offs = (0x40 + (sprite * 4));

	    int ypos = (special_ram.at((sprite_offs + 3)) + 1);
	    int xpos = (special_ram.at(sprite_offs) - 16);

	    if (ypos < 8)
	    {
		continue;
	    }

	    uint8_t flip_attrib = (special_ram.at((sprite_offs + 1)));

	    bool is_yflip = testbit(flip_attrib, 6);
	    bool is_xflip = testbit(flip_attrib, 7);

	    if (sprite <= 2)
	    {
		xpos += 1;
	    }

	    if (!inRange(xpos, -16, width))
	    {
		continue;
	    }

	    uint8_t sprite_num = (flip_attrib & 0x3F);
	    int color = (special_ram.at((sprite_offs + 2)) & 0x7);
	    auto palette = get_palette(color);
	    draw_sprite(sprite_num, xpos, ypos, is_xflip, is_yflip, palette);
	}
    }

    drivergalaxian::drivergalaxian()
    {
	galaxian_proc = new BerrnZ80Processor(3072000, inter);
	galaxian_cpu = new BerrnCPU(scheduler, *galaxian_proc);

	vblank_timer = new BerrnTimer("VBlank", scheduler, [&](int64_t, int64_t) {
	    inter.updatePixels();
	});

	star_scroll_timer = new BerrnTimer("Starfield", scheduler, [&](int64_t, int64_t) {
	    inter.star_scroll_callback();
	});

	interrupt_timer = new BerrnTimer("Interrupt", scheduler, [&](int64_t param, int64_t) {
	    if (param == 1)
	    {
		interrupt_timer->start(16500, true);
		star_scroll_timer->start(16500, true);
	    }

	    this->interrupt_handler();
	});
    }

    drivergalaxian::~drivergalaxian()
    {

    }

    string drivergalaxian::drivername()
    {
	return "galaxian";
    }

    bool drivergalaxian::hasdriverROMs()
    {
	return true;
    }

    void drivergalaxian::loadROMs()
    {
	loadROM("galmidw.u", 0x0000, 0x0800, inter.get_gamerom());
	loadROM("galmidw.v", 0x0800, 0x0800, inter.get_gamerom());
	loadROM("galmidw.w", 0x1000, 0x0800, inter.get_gamerom());
	loadROM("galmidw.y", 0x1800, 0x0800, inter.get_gamerom());
	loadROM("7l", 0x2000, 0x0800, inter.get_gamerom());
	loadROM("1h.bin", 0x0000, 0x0800, inter.get_tilerom());
	loadROM("1k.bin", 0x0800, 0x0800, inter.get_tilerom());
	loadROM("6l.bpr", 0x0000, 0x0020, inter.get_palrom());
    }

    void drivergalaxian::interrupt_handler()
    {
	if (inter.irq_enabled())
	{
	    galaxian_proc->fire_nmi();
	}
    }

    bool drivergalaxian::drvinit()
    {
	loadROMs();
	scheduler.reset();
	scheduler.add_device(galaxian_cpu);
	interrupt_timer->start(14000, false, 1);
	vblank_timer->start(16500, true);
	galaxian_proc->init();
	inter.init();
	resize(224, 256, 2);
	return isallfilesloaded();
    }

    void drivergalaxian::drvshutdown()
    {
	vblank_timer->stop();
	inter.shutdown();
	galaxian_proc->shutdown();
	scheduler.remove_timer(vblank_timer);
	scheduler.remove_timer(interrupt_timer);
	scheduler.remove_device(galaxian_cpu);
    }
  
    void drivergalaxian::drvrun()
    {
	int64_t schedule_time = scheduler.get_current_time();

	int64_t frame_time = 16500;

	while (scheduler.get_current_time() < (schedule_time + frame_time))
	{
	    scheduler.timeslice();
	}

	filltexrect(0, 0, 224, 256, inter.get_framebuffer());
    }

    float drivergalaxian::get_framerate()
    {
	return (16000.0 / 132.0 / 2.0); // Framerate is 60.606060 Hz
    }

    void drivergalaxian::keychanged(BerrnInput key, bool is_pressed)
    {
	string key_state = (is_pressed) ? "pressed" : "released";

	switch (key)
	{
	    case BerrnInput::BerrnCoin:
	    {
		cout << "Coin button has been " << key_state << endl;
		inter.coin(is_pressed);
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
	    case BerrnInput::BerrnFireP1:
	    {
		cout << "P1 fire button has been " << key_state << endl;
	    }
	    break;
	    default: break;
	}
    }
};