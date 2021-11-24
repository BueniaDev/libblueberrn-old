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

#ifndef BERRN_PACMAN
#define BERRN_PACMAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <audio/wsg3.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API PacmanInterface : public BerrnInterface
    {
	public:
	    PacmanInterface();
	    ~PacmanInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t readOp8(uint16_t addr);
	    void portOut(uint16_t port, uint8_t val);
	    void updatePixels();

	    vector<uint8_t> &get_gamerom()
	    {
		return gamerom;
	    }

	    vector<uint8_t> &get_color_rom()
	    {
		return color_rom;
	    }

	    vector<uint8_t> &get_pal_rom()
	    {
		return pal_rom;
	    }

	    vector<uint8_t> &get_tile_rom()
	    {
		return tile_rom;
	    }

	    vector<uint8_t> &get_sprite_rom()
	    {
		return sprite_rom;
	    }

	    vector<uint8_t> &get_sound_rom()
	    {
		return sound_rom;
	    }

	    bool is_vblank_enabled()
	    {
		return vblank_enable;
	    }

	    bool is_sound_enabled()
	    {
		return sound_enable;
	    }

	    uint8_t get_int_vec()
	    {
		return int_vector;
	    }

	    array<berrnRGBA, (288 * 224)> get_framebuffer() const
	    {
		return framebuffer;
	    }

	    uint32_t get_wsg3_sample_rate()
	    {
		return pac_sound.get_sample_rate();
	    }

	    void clock_wsg3()
	    {
		pac_sound.clockchip();
	    }

	    array<int16_t, 2> get_wsg3_sample()
	    {
		return pac_sound.get_sample();
	    }

	    void coin(bool is_pressed);
	    void p1up(bool is_pressed);
	    void p1down(bool is_pressed);
	    void p1left(bool is_pressed);
	    void p1right(bool is_pressed);
	    void p1start(bool is_pressed);

	private:
	    uint8_t readByte(uint16_t addr);

	    void writeIO(int addr, uint8_t data);

	    array<uint8_t, 4> get_palette(int pal_num);

	    void decode_images();
	    void decode_strip(const uint8_t *src, uint8_t *dst, int bx, int by, int width);

	    void draw_tile(uint8_t tile_num, array<uint8_t, 4> palette, int xcoord, int ycoord);
	    void draw_sprite(uint8_t sprite_num, array<uint8_t, 4> palette, int xcoord, int ycoord, bool flipx, bool flipy);
	    void set_pixel(int xpos, int ypos, uint8_t color_num);

	    vector<uint8_t> gamerom;
	    array<uint8_t, 0x400> vram;
	    array<uint8_t, 0x400> cram;
	    array<uint8_t, 0x3F0> mainram;
	    array<uint8_t, 0x10> oam;
	    array<uint8_t, 0x10> sprite_pos;
	    vector<uint8_t> color_rom;
	    vector<uint8_t> pal_rom;
	    vector<uint8_t> tile_rom;
	    vector<uint8_t> sprite_rom;
	    vector<uint8_t> tile_ram;
	    vector<uint8_t> sprite_ram;
	    vector<uint8_t> sound_rom;
	    bool vblank_enable = false;
	    bool sound_enable = false;
	    bool flip_screen = false;
	    array<berrnRGBA, (288 * 224)> framebuffer;

	    uint8_t int_vector = 0;
	    uint8_t in0_val = 0;
	    uint8_t in1_val = 0;

	    wsg3audio pac_sound;
    };

    class LIBBLUEBERRN_API driverpacman : public berrndriver
    {
	public:
	    driverpacman();
	    ~driverpacman();

	    string drivername();
	    bool hasdriverROMs();

	    virtual void loadROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    PacmanInterface inter;
	    BerrnScheduler scheduler;

	    void interrupt_handler();
	    void sound_clock();

	    BerrnZ80Processor *pacman_proc = NULL;
	    BerrnCPU *pacman_cpu = NULL;

	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *sound_timer = NULL;

	    float out_step = 0.0f;
	    float in_step = 0.0f;
	    float out_time = 0.0f;
    };
};


#endif // BERRN_PACMAN