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

#ifndef BERRN_GALAGA
#define BERRN_GALAGA

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <video/galaga.h>
#include <audio/wsg.h>
#include <audio/namco54.h>
#include <machine/namco06.h>
#include <machine/namco51.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class GalagaCore;

    class GalagaInterface
    {
	public:
	    GalagaInterface(GalagaCore &core);
	    ~GalagaInterface();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	private:
	    GalagaCore &parent_core;
    };

    class GalagaCPUInterface : public BerrnInterface
    {
	public:
	    GalagaCPUInterface(string tag_str, berrndriver &drv, GalagaInterface &inter);
	    ~GalagaCPUInterface();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    uint8_t readOp8(uint16_t addr);

	private:
	    string tag;
	    berrndriver &driver;
	    GalagaInterface &interface;

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    vector<uint8_t> cpu_rom;
    };

    class GalagaCore
    {
	public:
	    GalagaCore(berrndriver &drv);
	    ~GalagaCore();

	    bool init_core();
	    void shutdown_core();
	    void run_core();
	    void key_changed(BerrnInput key, bool is_pressed);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    BerrnScheduler scheduler;

	    void chip_select_galaga(int addr, bool line);
	    void rw_galaga(int addr, bool line);
	    uint8_t read_galaga(int addr);
	    void write_galaga(int addr, uint8_t data);

	    int64_t time_until_scanline(int scanline);

	    uint8_t read_io_ports(int addr);

	    void write_latch(int addr, uint8_t data);

	    void fire_main_irq();
	    void fire_aux_irq();
	    void fire_51xx_irq();

	    bool main_irq_enabled = false;
	    bool aux_irq_enabled = false;
	    bool sound_irq_enabled = false;

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *interrupt_timer = NULL;
	    BerrnTimer *sound_nmi_timer = NULL;
	    BerrnTimer *sound_timer = NULL;

	    GalagaInterface *shared_inter = NULL;

	    GalagaCPUInterface *main_inter = NULL;
	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    GalagaCPUInterface *aux_inter = NULL;
	    BerrnZ80Processor *aux_proc = NULL;
	    BerrnCPU *aux_cpu = NULL;

	    GalagaCPUInterface *sound_inter = NULL;
	    BerrnZ80Processor *sound_proc = NULL;
	    BerrnCPU *sound_cpu = NULL;

	    namco06xx *namco_06xx = NULL;
	    namco51xx *namco_51xx = NULL;
	    namco54xx *namco_54xx = NULL;

	    array<uint8_t, 0x400> main_ram1;
	    array<uint8_t, 0x400> main_ram2;
	    array<uint8_t, 0x400> main_ram3;

	    int64_t vblank_start_time = 0;

	    wsg3device *wsg3_device = NULL;

	    uint8_t dsw_a = 0x00;
	    uint8_t dsw_b = 0x00;

	    uint8_t port0_val = 0;
	    uint8_t port1_val = 0;

	    galagavideo *video_core = NULL;
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
	    GalagaCore *core = NULL;
    };
};


#endif // BERRN_GALAGA