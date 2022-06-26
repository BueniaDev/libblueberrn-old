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

#ifndef BERRN_KONMEDAL
#define BERRN_KONMEDAL

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/zilogz80.h>
#include <video/konmedal.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API BerrnShuriBoy : public BerrnInterface
    {
	public:
	    BerrnShuriBoy(berrndriver &drv);
	    ~BerrnShuriBoy();

	    bool init_core();
	    void stop_core();
	    void run_core();
	    void key_changed(BerrnInput key, bool is_pressed);

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	protected:
	    virtual void init_ram();

	    virtual uint8_t readDIP(int bank);
	    virtual void updatePixels();

	    void writeRAM(uint16_t addr, uint8_t data)
	    {
		addr &= 0x7FF;
		main_ram.at(addr) = data;
	    }

	    shuriboyvideo *tile_video = NULL;

	private:
	    berrndriver &driver;

	    BerrnZ80CPU *main_cpu = NULL;

	    int64_t vblank_start_time = 0;

	    uint8_t in2_port = 0;

	    int vpos();

	    void scanline_callback(int vpos);

	    int64_t time_until_pos(int vpos);

	    void vblank();

	    bool is_first_time = false;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x800> main_ram;

	    array<uint8_t, 0x100> scc_ram;

	    bool is_irq_enabled = false;
	    bool is_nmi_enabled = false;

	    BerrnTimer *vblank_timer = NULL;
	    BerrnTimer *vblank_end_timer = NULL;
	    BerrnTimer *irq_timer = NULL;

	    int current_scanline = 0;

	    int current_rom_bank = 0;
    };

    class LIBBLUEBERRN_API BerrnMarioRoulette : public BerrnShuriBoy
    {
	public:
	    BerrnMarioRoulette(berrndriver &drv);
	    ~BerrnMarioRoulette();

	    void init_ram();

	    uint8_t readDIP(int bank);
	    void updatePixels();
    };

    class LIBBLUEBERRN_API drivershuriboy : public berrndriver
    {
	public:
	    drivershuriboy();
	    ~drivershuriboy();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    BerrnShuriBoy *core = NULL;
    };

    class LIBBLUEBERRN_API drivermariorou : public berrndriver
    {
	public:
	    drivermariorou();
	    ~drivermariorou();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    BerrnMarioRoulette *core = NULL;
    };
};


#endif // BERRN_KONMEDAL