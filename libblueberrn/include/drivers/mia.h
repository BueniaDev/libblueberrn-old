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

#ifndef BERRN_MIA
#define BERRN_MIA

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/motorola68k.h>
#include <cpu/zilogz80.h>
#include <video/mia.h>
#include <audio/ym2151.h>
#include <konami/k007232.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class MIACore;

    class LIBBLUEBERRN_API MIAM68K : public BerrnInterface
    {
	public:
	    MIAM68K(berrndriver &drv, MIACore &core);
	    ~MIAM68K();

	    void init();
	    void shutdown();

	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);
	    void writeCPU16(bool upper, bool lower, uint32_t addr, uint16_t data);

	private:
	    berrndriver &driver;
	    MIACore &main_core;

	    vector<uint8_t> main_rom;
	    array<array<uint8_t, 0x4000>, 2> main_ram;
    };

    class LIBBLUEBERRN_API MIAZ80 : public BerrnInterface
    {
	public:
	    MIAZ80(berrndriver &drv, MIACore &core);
	    ~MIAZ80();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	    void processAudio();

	private:
	    berrndriver &driver;
	    MIACore &sound_core;

	    ym2151device *opm = NULL;
	    k007232device *k007232 = NULL;

	    vector<uint8_t> sound_rom;
	    array<uint8_t, 0x800> sound_ram;
    };

    class LIBBLUEBERRN_API MIACore
    {
	public:
	    MIACore(berrndriver &drv);
	    ~MIACore();

	    bool init_core();
	    void stop_core();
	    void run_core();
	    void key_changed(BerrnInput key, bool is_pressed);

	    void process_audio();

	    uint16_t readk052109_noA12(bool upper, bool lower, uint32_t addr);
	    void writek052109_noA12(bool upper, bool lower, uint32_t addr, uint16_t data);

	    uint8_t k051960_read(uint16_t addr);
	    void k051960_write(uint16_t addr, uint8_t data);

	    uint8_t readPalette(uint32_t addr);
	    void writePalette(uint32_t addr, uint8_t data);

	    void write0A0000(uint16_t data);

	    uint8_t readSoundLatch();
	    void writeSoundLatch(uint8_t data);

	    uint8_t readDIP(int reg);

	private:
	    berrndriver &driver;

	    MIAM68K *main_inter = NULL;
	    BerrnM68KCPU *main_cpu = NULL;

	    MIAZ80 *sound_inter = NULL;
	    BerrnZ80CPU *sound_cpu = NULL;

	    miavideo *video = NULL;

	    BerrnTimer *vblank_timer = NULL;

	    bool is_irq_enabled = false;

	    bool prev_int = false;

	    uint8_t coins_port = 0;

	    uint8_t p1_port = 0;

	    uint8_t sound_cmd = 0;
    };

    class LIBBLUEBERRN_API drivermia : public berrndriver
    {
	public:
	    drivermia();
	    ~drivermia();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void process_audio();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    MIACore *core = NULL;
    };
};


#endif // BERRN_MIA