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

#ifndef BERRN_GRADIUS
#define BERRN_GRADIUS

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/motorola68k.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class GradiusCore;

    class LIBBLUEBERRN_API GradiusM68K : public BerrnInterface
    {
	public:
	    GradiusM68K(berrndriver &drv, GradiusCore &core);
	    ~GradiusM68K();

	    void init();
	    void shutdown();

	    uint16_t readCPU16(bool upper, bool lower, uint32_t addr);

	private:
	    berrndriver &driver;
	    GradiusCore &main_core;

	    vector<uint8_t> main_rom;
    };

    class LIBBLUEBERRN_API GradiusCore
    {
	public:
	    GradiusCore(berrndriver &drv);
	    ~GradiusCore();

	    bool init_core();
	    void stop_core();
	    void run_core();

	private:
	    berrndriver &driver;

	    GradiusM68K *main_inter = NULL;
	    BerrnM68KCPU *main_cpu = NULL;
    };

    class LIBBLUEBERRN_API drivergradius : public berrndriver
    {
	public:
	    drivergradius();
	    ~drivergradius();

	    string drivername();
	    string parentname();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void keychanged(BerrnInput key, bool is_pressed);

	private:
	    GradiusCore *core = NULL;
    };
};


#endif // BERRN_GRADIUS