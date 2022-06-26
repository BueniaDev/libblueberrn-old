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

#ifndef BERRN_CENTIPEDE
#define BERRN_CENTIPEDE

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/mos6502.h>
#include <machine/er2055.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API BerrnCentipedeBase : public BerrnInterface
    {
	public:
	    BerrnCentipedeBase(berrndriver &drv);
	    ~BerrnCentipedeBase();

	    bool initcore();
	    void stopcore();
	    void runcore();

	    uint8_t readCPU8(uint16_t addr);
	    void writeCPU8(uint16_t addr, uint8_t data);

	    virtual bool isValidAddr(uint16_t addr, bool is_write);
	    virtual uint8_t readMem(uint16_t addr);
	    virtual void writeMem(uint16_t addr, uint8_t data);

	    virtual void writeLatchUpper(int addr, bool line);

	    void writeLatch(int addr, uint8_t data);

	private:
	    berrndriver &driver;

	    vector<uint8_t> main_rom;
	    array<uint8_t, 0x400> main_ram;
	    array<uint8_t, 0x400> temp_vram;

	    Berrn6502CPU *main_cpu = NULL;

	    BerrnTimer *irq_timer = NULL;

	    int current_scanline = 0;

	    uint8_t earom_read();
	    void earom_write(int addr, uint8_t data);
	    void earom_control_write(uint8_t data);

	    er2055core earom;
    };

    class LIBBLUEBERRN_API BerrnCentipede : public BerrnCentipedeBase
    {
	public:
	    BerrnCentipede(berrndriver &drv);
	    ~BerrnCentipede();

	    bool isValidAddr(uint16_t addr, bool is_write);
	    uint8_t readMem(uint16_t addr);
	    void writeMem(uint16_t addr, uint8_t data);

	    void writeLatchUpper(int addr, bool line);
    };

    class LIBBLUEBERRN_API drivercentiped : public berrndriver
    {
	public:
	    drivercentiped();
	    ~drivercentiped();

	    string drivername();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    BerrnCentipede *core = NULL;
    };
};

#endif // BERRN_CENTIPEDE