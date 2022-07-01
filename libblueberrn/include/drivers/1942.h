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

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x1000> main_ram;

	    int current_rom_bank = 0;
    };

    class LIBBLUEBERRN_API Berrn1942Sound : public BerrnInterface
    {
	public:
	    Berrn1942Sound(berrndriver &drv, Berrn1942Core &core);
	    ~Berrn1942Sound();

	    void init();
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	    void processAudio();

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
	    void key_changed(BerrnInput key, bool is_pressed);

	    void process_audio();

	    uint8_t readGraphics(int bank, uint16_t addr);
	    void writeGraphics(int bank, uint16_t addr, uint8_t data);

	    void writeIO(int addr, uint8_t data);

	    uint8_t readDIP(int reg);

	    uint8_t readSoundLatch();
	    void writeSoundLatch(uint8_t data);

	private:
	    berrndriver &driver;

	    Berrn1942Main *main_inter = NULL;
	    BerrnZ80CPU *main_cpu = NULL;

	    Berrn1942Sound *sound_inter = NULL;
	    BerrnZ80CPU *sound_cpu = NULL;

	    berrnscanlinetimer *timer = NULL;

	    berrn1942video *video = NULL;

	    void scanline_callback(int vpos);

	    void writeC804(uint8_t data);

	    uint8_t sound_cmd = 0;
	    uint8_t system_reg = 0;
	    uint8_t p1_reg = 0;
    };

    class LIBBLUEBERRN_API driver1942 : public berrndriver
    {
	public:
	    driver1942();
	    ~driver1942();

	    string drivername();
	    uint32_t get_flags();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();
	    void keychanged(BerrnInput key, bool is_pressed);

	    void process_audio();

	private:
	    Berrn1942Core *core = NULL;
    };
};


#endif // BERRN_1942