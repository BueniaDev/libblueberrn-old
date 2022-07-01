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

#ifndef BERRN_BOMBJACK
#define BERRN_BOMBJACK

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <audio/ay8910.h>
#include <video/bombjack.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class BombJackCore;

    class LIBBLUEBERRN_API BombJackMain : public BerrnInterface
    {
	public:
	    BombJackMain(berrndriver &drv, BombJackCore &core);
	    ~BombJackMain();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    BombJackCore &main_core;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x1000> main_ram;
    };

    class LIBBLUEBERRN_API BombJackSound : public BerrnInterface
    {
	public:
	    BombJackSound(berrndriver &drv, BombJackCore &core);
	    ~BombJackSound();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);
	    void portOut(uint16_t port, uint8_t data);
	    
	    void processAudio();

	private:
	    berrndriver &driver;
	    BombJackCore &sound_core;

	    vector<uint8_t> sound_rom;
	    array<uint8_t, 0x400> sound_ram;

	    ay8910device *ay1 = NULL;
	    ay8910device *ay2 = NULL;
	    ay8910device *ay3 = NULL;
    };

    class LIBBLUEBERRN_API BombJackCore
    {
	public:
	    BombJackCore(berrndriver &drv);
	    ~BombJackCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    void process_audio();

	    void writeNMI(uint8_t data);

	    uint8_t readGraphics(int bank, uint16_t addr);
	    void writeGraphics(int bank, uint16_t addr, uint8_t data);

	    void writeBackground(uint8_t data);

	    uint8_t readSoundLatch();
	    void writeSoundLatch(uint8_t data);

	private:
	    berrndriver &driver;

	    BombJackMain *main_inter = NULL;
	    BerrnZ80CPU *main_cpu = NULL;

	    BombJackSound *sound_inter = NULL;
	    BerrnZ80CPU *sound_cpu = NULL;

	    BerrnTimer *vblank_timer = NULL;

	    bombjackvideo *video = NULL;

	    bool is_nmi_enabled = false;

	    uint8_t sound_cmd = 0x00;
    };

    class LIBBLUEBERRN_API driverbombjack : public berrndriver
    {
	public:
	    driverbombjack();
	    ~driverbombjack();

	    string drivername();
	    uint32_t get_flags();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void process_audio();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    BombJackCore *core = NULL;
    };
};


#endif // BERRN_BOMBJACK