/*
    This file is part of libblueberrn.
    Copyright (C) 2021 BueniaDev.

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

#include "galaxian.h"
using namespace berrn;

namespace berrn
{
    GalaxianInterface::GalaxianInterface()
    {

    }

    GalaxianInterface::~GalaxianInterface()
    {

    }

    void GalaxianInterface::init()
    {
	gamerom.resize(0x4000, 0);
	core.setinterface(this);
	core.init();
    }

    void GalaxianInterface::shutdown()
    {
	core.shutdown();
	gamerom.clear();
    }

    void GalaxianInterface::reset()
    {
	core.reset();
    }

    void GalaxianInterface::run()
    {
	while (totalcycles < cycles_per_frame)
	{
	    totalcycles += core.runinstruction();
	}

	totalcycles = 0;
    }

    uint8_t GalaxianInterface::readByte(uint16_t addr)
    {
	addr &= 0x7FFF;
	uint8_t temp = 0;
	if (addr < 0x4000)
	{
	    temp = gamerom[addr];
	}
	else
	{
	    cout << "Reading byte from address of " << hex << (int)addr << endl;
	    exit(0);
	    temp = 0;
	}

	return temp;
    }

    void GalaxianInterface::writeByte(uint16_t addr, uint8_t val)
    {
	cout << "Writing byte of " << hex << (int)val << " to address of " << hex << (int)addr << endl;
	exit(0);
    }

    uint8_t GalaxianInterface::portIn(uint8_t port)
    {
	cout << "Reading byte from port of " << hex << (int)port << endl;
	exit(0);
	return 0;
    }

    void GalaxianInterface::portOut(uint8_t port, uint8_t val)
    {
	cout << "Writing byte of " << hex << (int)val << " to port of " << hex << (int)port << endl;
	exit(0);
    }

    drivergalaxian::drivergalaxian()
    {

    }

    drivergalaxian::~drivergalaxian()
    {

    }

    string drivergalaxian::drivername()
    {
	return "galaxian";
    }

    bool drivergalaxian::hasdriverROMs()
    {
	return true;
    }

    bool drivergalaxian::drvinit()
    {
	loadROM("galmidw.u", 0x0000, 0x0800, inter.gamerom);
	loadROM("galmidw.v", 0x0800, 0x0800, inter.gamerom);
	loadROM("galmidw.w", 0x1000, 0x0800, inter.gamerom);
	loadROM("galmidw.y", 0x1800, 0x0800, inter.gamerom);
	loadROM("7l", 0x2000, 0x0800, inter.gamerom);
	inter.init();
	resize(640, 480, 1);
	return isallfilesloaded();
    }

    void drivergalaxian::drvshutdown()
    {
	inter.shutdown();
    }
  
    void drivergalaxian::drvrun()
    {
	inter.run();
	fillrect(0, 0, 640, 480, red());
    }

    void drivergalaxian::drvcoin(bool pressed)
    {
	if (pressed)
	{
	    cout << "Coin button pressed" << endl;
	}
    }

    void drivergalaxian::drvstartp1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Start button pressed" << endl;
	}
    }

    void drivergalaxian::drvleftp1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Left button pressed" << endl;
	}
    }

    void drivergalaxian::drvrightp1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Right button pressed" << endl;
	}
    }

    void drivergalaxian::drvfirep1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Fire button pressed" << endl;
	}
    }
};