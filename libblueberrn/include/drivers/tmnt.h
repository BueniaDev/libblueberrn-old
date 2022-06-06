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

#ifndef BERRN_TMNT
#define BERRN_TMNT

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/motorola68k.h>
#include <cpu/zilogz80.h>
#include <video/tmnt.h>
#include <audio/ym2151.h>
#include <konami/k007232.h>
#include <audio/samples.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class TMNTCore;

    class LIBBLUEBERRN_API TMNTM68K : public BerrnInterface
    {
	public:
	    TMNTM68K(berrndriver &drv, TMNTCore &core);
	    ~TMNTM68K();

	    void init();
	    void shutdown();

	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);
	    void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x4000> main_ram;
	    berrndriver &driver;
	    TMNTCore &main_core;
    };

    class LIBBLUEBERRN_API TMNTZ80 : public BerrnInterface
    {
	public:
	    TMNTZ80(berrndriver &drv, TMNTCore &core);
	    ~TMNTZ80();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	    void processAudio();

	private:
	    berrndriver &driver;
	    TMNTCore &sound_core;

	    void initTitle();

	    vector<uint8_t> sound_rom;
	    array<uint8_t, 0x800> sound_ram;

	    vector<int16_t> title_samples;

	    ym2151device *opm = NULL;
	    k007232device *k007232 = NULL;
	    samplesdevice *title = NULL;

	    uint8_t internal_latch = 0;

	    int title_id = 0;
    };

    class LIBBLUEBERRN_API TMNTCore : public BerrnInterface
    {
	public:
	    TMNTCore(berrndriver &drv);
	    ~TMNTCore();

	    bool init_core();
	    void stop_core();
	    void run_core();
	    void key_changed(BerrnInput key, bool is_pressed);

	    void process_audio();

	    uint8_t readPalette(uint32_t addr);
	    void writePalette(uint32_t addr, uint8_t data);
	    void write0A0000(uint16_t data);
	    uint16_t readk052109_noA12(bool upper, bool lower, uint32_t addr);
	    void writek052109_noA12(bool upper, bool lower, uint32_t addr, uint16_t data);

	    uint8_t k051960_read(uint16_t addr);
	    void k051960_write(uint16_t addr, uint8_t data);

	    void writePriority(uint8_t data);

	    uint8_t readSoundLatch();
	    void writeSoundLatch(uint8_t data);

	    uint8_t readDIP(int bank);

	private:
	    berrndriver &driver;

	    int64_t vblank_start_time = 0;
	    int64_t time_until_pos(int vpos);

	    TMNTM68K *main_inter = NULL;
	    BerrnM68KProcessor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    TMNTZ80 *sound_inter = NULL;
	    BerrnZ80Processor *sound_proc = NULL;
	    BerrnCPU *sound_cpu = NULL;

	    tmntvideo *video = NULL;
	    BerrnTimer *vblank_timer = NULL;

	    bool is_irq_enabled = false;

	    bool prev_int = false;

	    uint8_t coins_port = 0;
	    uint8_t p1_port = 0;

	    uint8_t sound_cmd = 0;
    };

    class LIBBLUEBERRN_API drivertmnt : public berrndriver
    {
	public:
	    drivertmnt();
	    ~drivertmnt();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void process_audio();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    TMNTCore *core = NULL;
    };
};

#endif // BERRN_TMNT