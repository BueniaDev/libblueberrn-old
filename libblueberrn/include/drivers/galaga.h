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

#ifndef BERRN_GALAGA
#define BERRN_GALAGA

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <machine/namco06.h>
#include <machine/namco51.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    enum CPUType : int
    {
	Main = 0,
	Aux = 1,
	Sound = 2,
    };

    class GalagaCore;

    class LIBBLUEBERRN_API GalagaInterface
    {
	public:
	    GalagaInterface(GalagaCore &core);
	    ~GalagaInterface();

	    void init();
	    void shutdown();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    bool isValidAddr(uint16_t addr);

	    void updatePixels();

	    BerrnBitmapRGB *getBitmap()
	    {
		return bitmap;
	    }

	    void set_tile_rom(vector<uint8_t> rom_vec)
	    {
		tile_rom = vector<uint8_t>(rom_vec.begin(), rom_vec.end());
	    }

	    void set_pal_rom(vector<uint8_t> rom_vec)
	    {
		pal_rom = vector<uint8_t>(rom_vec.begin(), rom_vec.end());
	    }

	    void set_color_rom(vector<uint8_t> rom_vec)
	    {
		color_rom = vector<uint8_t>(rom_vec.begin(), rom_vec.end());
	    }

	private:
	    GalagaCore &parent_core;

	    array<uint8_t, 0x800> vram;
	    array<uint8_t, 0x400> ram1;
	    array<uint8_t, 0x400> ram2;
	    array<uint8_t, 0x400> ram3;

	    void decode_images();
	    void decode_strip(const uint8_t *src, uint8_t *dst, int bx, int by, int width);

	    void draw_tile(uint8_t tile_num, array<uint8_t, 4> palette, int xcoord, int ycoord);

	    void set_pixel(int xpos, int ypos, uint8_t color_num);
	    array<uint8_t, 4> get_palette(int pal_num);

	    uint8_t dsw_a = 0;
	    uint8_t dsw_b = 0;

	    vector<uint8_t> tile_rom;
	    vector<uint8_t> tile_ram;
	    vector<uint8_t> pal_rom;
	    vector<uint8_t> color_rom;

	    BerrnBitmapRGB *bitmap = NULL;
    };

    class LIBBLUEBERRN_API GalagaCPU : public BerrnInterface
    {
	public:
	    GalagaCPU(GalagaInterface *cb);
	    ~GalagaCPU();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t readOp8(uint16_t addr);

	    void set_core_rom(vector<uint8_t> rom_vec)
	    {
		core_rom = vector<uint8_t>(rom_vec.begin(), rom_vec.end());
	    }

	private:
	    vector<uint8_t> core_rom;

	    GalagaInterface *inter = NULL;

	    uint8_t readByte(uint16_t addr);
    };

    class LIBBLUEBERRN_API GalagaCore : public BerrnInterface
    {
	public:
	    GalagaCore(berrndriver &drv);
	    ~GalagaCore();

	    void init();
	    void shutdown();

	    namco06xx *get_namco_06xx()
	    {
		return namco_06xx;
	    }

	    void writeLatch(int offset, uint8_t data);

	    BerrnBitmapRGB *getBitmap()
	    {
		return shared_inter->getBitmap();
	    }

	    void set_rom(CPUType cpu_type, vector<uint8_t> rom_vec)
	    {
		switch (cpu_type)
		{
		    case Main: main_inter->set_core_rom(rom_vec); break;
		    case Aux: aux_inter->set_core_rom(rom_vec); break;
		    case Sound: sound_inter->set_core_rom(rom_vec); break;
		    default: break;
		}
	    }

	    void set_tile_rom(vector<uint8_t> rom_vec)
	    {
		shared_inter->set_tile_rom(rom_vec);
	    }

	    void set_pal_rom(vector<uint8_t> rom_vec)
	    {
		shared_inter->set_pal_rom(rom_vec);
	    }

	    void set_color_rom(vector<uint8_t> rom_vec)
	    {
		shared_inter->set_color_rom(rom_vec);
	    }

	    void run_core();
	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    BerrnScheduler scheduler;
	    GalagaCPU *main_inter = NULL;
	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    GalagaCPU *aux_inter = NULL;
	    BerrnZ80Processor *aux_proc = NULL;
	    BerrnCPU *aux_cpu = NULL;

	    GalagaCPU *sound_inter = NULL;
	    BerrnZ80Processor *sound_proc = NULL;
	    BerrnCPU *sound_cpu = NULL;

	    GalagaInterface *shared_inter = NULL;

	    namco06xx *namco_06xx = NULL;
	    namco51xx *namco_51xx = NULL;

	    bool aux_irq_enabled = false;
	    bool sound_irq_enabled = false;

	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *vblank_timer = NULL;

	    berrndriver &driver;
    };

    class LIBBLUEBERRN_API drivergalaga : public berrndriver
    {
	public:
	    drivergalaga();
	    ~drivergalaga();

	    string drivername();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    GalagaCore *galaga_core = NULL;
	    bool loadROMs();
    };
};


#endif // BERRN_GALAGA