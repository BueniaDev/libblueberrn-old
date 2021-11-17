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

#ifndef BERRN_GALAXIAN
#define BERRN_GALAXIAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API GalaxianInterface : public BerrnInterface
    {
	public:
	    GalaxianInterface();
	    ~GalaxianInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t readOp8(uint16_t addr);
	    void updatePixels();

	    vector<uint8_t> &get_gamerom()
	    {
		return gamerom;
	    }

	    vector<uint8_t> &get_tilerom()
	    {
		return tilerom;
	    }

	    vector<uint8_t> &get_palrom()
	    {
		return palrom;
	    }

	    array<berrnRGBA, (256 * 224)> get_framebuffer() const
	    {
		return framebuffer;
	    }

	    void star_scroll_callback();

	    bool irq_enabled()
	    {
		return is_irq_enabled;
	    }

	private:
	    vector<uint8_t> char_data;
	    vector<uint8_t> sprite_data;

	    array<uint8_t, 4> get_palette(int color);
	    void draw_tile(uint8_t tile_num, int x, int y, array<uint8_t, 4> palette);
	    void draw_sprite(uint8_t sprite_num, int xpos, int ypos, bool flip_x, bool flip_y, array<uint8_t, 4> palette);
	    void set_pixel(int xpos, int ypos, uint8_t color);
	    void decode_images();
	    // TODO: Implement native tile decoding in core driver API
	    void decode_char(const uint8_t *src, uint8_t *dst, int ox, int oy, int width);

	    // Tile and sprite decoding functions
	    void decode_tile(const uint8_t *src, uint8_t *dst);
	    void decode_sprite(const uint8_t *src, uint8_t *dst);

	    void init_starfield();
	    void draw_starfield();

	    void set_raw_pixel(int xpos, int ypos, berrnRGBA color);

	    struct StarfieldItem
	    {
		int xpos = 0;
		int ypos = 0;
		int color = 0;
	    };

	    array<StarfieldItem, 2520> starfield;

	    void draw_star_pixel(int xpos, int ypos, int color);

	    uint8_t readByte(uint16_t addr);
	    void writeIOupper(uint16_t addr, uint8_t data);

	    vector<uint8_t> gamerom;
	    array<uint8_t, 0x400> gameram;
	    vector<uint8_t> tilerom;
	    vector<uint8_t> palrom;

	    array<uint8_t, 0x400> vram;
	    array<uint8_t, 0x100> special_ram;
	    array<berrnRGBA, (256 * 224)> framebuffer;

	    bool is_irq_enabled = false;
	    bool is_stars_enabled = false;
	    int starfield_scroll_pos = 0;

	    const int width = 224;
	    const int height = 256;
    };

    class LIBBLUEBERRN_API drivergalaxian : public berrndriver
    {
	public:
	    drivergalaxian();
	    ~drivergalaxian();

	    string drivername();
	    bool hasdriverROMs();

	    virtual void loadROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	    void interrupt_handler();

	private:
	    GalaxianInterface inter;
	    BerrnScheduler scheduler;

	    BerrnZ80Processor *galaxian_proc = NULL;
	    BerrnCPU *galaxian_cpu = NULL;

	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *star_scroll_timer = NULL;
    };
};


#endif // BERRN_GALAXIAN