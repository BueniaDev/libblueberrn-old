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
#include <machine/namco06.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class GalagaCore;

    class LIBBLUEBERRN_API GalagaInterface : public BerrnInterface
    {
	public:
	    GalagaInterface(berrndriver &drv, GalagaCore &core);
	    ~GalagaInterface();

	    void init(string tag);
	    void shutdown();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;
	    GalagaCore &shared_core;

	    string tag_str;

	    vector<uint8_t> main_rom;
    };

    class LIBBLUEBERRN_API GalagaCore
    {
	public:
	    GalagaCore(berrndriver &drv);
	    ~GalagaCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	private:
	    berrndriver &driver;

	    GalagaInterface *main_inter = NULL;
	    BerrnZ80CPU *main_cpu = NULL;

	    GalagaInterface *aux_inter = NULL;
	    BerrnZ80CPU *aux_cpu = NULL;

	    GalagaInterface *sound_inter = NULL;
	    BerrnZ80CPU *sound_cpu = NULL;

	    namco06xx *n06xx = NULL;

	    array<array<uint8_t, 0x400>, 3> main_ram;

	    galagavideo *video = NULL;

	    bool is_main_irq = false;
	    bool is_aux_irq = false;

	    BerrnTimer *vblank_timer = NULL;

	    uint8_t readRAM(int bank, uint16_t addr);
	    void writeRAM(int bank, uint16_t addr, uint8_t data);

	    void writeIO(int addr, bool line);

	    uint8_t dsw_a = 0;
	    uint8_t dsw_b = 0;
    };

    class LIBBLUEBERRN_API drivergalaga : public berrndriver
    {
	public:
	    drivergalaga();
	    ~drivergalaga();

	    string drivername();
	    uint32_t get_flags();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    GalagaCore *core = NULL;
    };
};


#endif // BERRN_GALAGA