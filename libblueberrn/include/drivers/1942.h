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

#ifndef BERRN_1942
#define BERRN_1942

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <audio/ay8910.h>
#include <video/1942.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class Berrn1942Core;

    class LIBBLUEBERRN_API Berrn1942Main : public BerrnInterface
    {
	public:
	    Berrn1942Main(berrndriver &drv, Berrn1942Core &core);
	    ~Berrn1942Main();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    Berrn1942Core &main_core;

	    int current_rom_bank = 0;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x1000> main_ram;
    };

    class LIBBLUEBERRN_API Berrn1942Sound : public BerrnInterface
    {
	public:
	    Berrn1942Sound(berrndriver &drv, Berrn1942Core &core);
	    ~Berrn1942Sound();

	    void init();
	    void shutdown();
	    void process_audio();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    Berrn1942Core &sound_core;

	    vector<uint8_t> sound_rom;
	    array<uint8_t, 0x800> sound_ram;

	    ay8910device *ay1 = NULL;
	    ay8910device *ay2 = NULL;
    };

    class LIBBLUEBERRN_API Berrn1942Core
    {
	public:
	    Berrn1942Core(berrndriver &drv);
	    ~Berrn1942Core();

	    bool init_core();
	    void stop_core();
	    void run_core();
	    void process_audio();

	    void key_changed(BerrnInput key, bool is_pressed);

	    uint8_t readDIP(int reg);

	    void writeIO(int reg, uint8_t data);

	    uint8_t readGraphics(int bank, uint16_t addr);
	    void writeGraphics(int bank, uint16_t addr, uint8_t data);

	    uint8_t readSoundLatch();

	private:
	    berrndriver &driver;

	    void writeSoundLatch(uint8_t data);
	    void writeC804(uint8_t data);

	    uint8_t p1_port = 0;

	    berrn1942video *video = NULL;

	    int vpos();
	    int64_t time_until_pos(int vpos);

	    void scanline_callback(int scanline);

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *irq_timer = NULL;

	    int64_t vblank_start_time = 0;
	    bool is_first_time = false;

	    Berrn1942Main *main_inter = NULL;
	    BerrnZ80Processor *main_proc = NULL;
	    BerrnCPU *main_cpu = NULL;

	    Berrn1942Sound *sound_inter = NULL;
	    BerrnZ80Processor *sound_proc = NULL;
	    BerrnCPU *sound_cpu = NULL;

	    uint8_t sound_cmd = 0;
    };

    class LIBBLUEBERRN_API driver1942 : public berrndriver
    {
	public:
	    driver1942();
	    ~driver1942();

	    string drivername();
	    uint32_t get_flags();

	    double get_framerate();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void process_audio();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    Berrn1942Core *core = NULL;
    };
};


#endif // BERRN_1942