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

#include "pacman.h"
using namespace berrn;

namespace berrn
{
    PacmanInterface::PacmanInterface()
    {

    }

    PacmanInterface::~PacmanInterface()
    {

    }

    void PacmanInterface::init()
    {
	core.setinterface(this);
	core.init();
    }

    void PacmanInterface::shutdown()
    {
	core.shutdown();
	gamerom.clear();
    }

    void PacmanInterface::reset()
    {
	core.reset();
    }

    void PacmanInterface::run()
    {
	while (totalcycles < cycles_per_frame)
	{
	    totalcycles += core.runinstruction();
	}

	totalcycles = 0;
    }

    uint8_t PacmanInterface::readByte(uint16_t addr)
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

    void PacmanInterface::writeByte(uint16_t addr, uint8_t val)
    {
	cout << "Writing byte of " << hex << (int)val << " to address of " << hex << (int)addr << endl;
	exit(0);
    }

    uint8_t PacmanInterface::portIn(uint8_t port)
    {
	cout << "Reading byte from port of " << hex << (int)port << endl;
	exit(0);
	return 0;
    }

    void PacmanInterface::portOut(uint8_t port, uint8_t val)
    {
	cout << "Writing byte of " << hex << (int)val << " to port of " << hex << (int)port << endl;
	exit(0);
    }

    driverpacman::driverpacman()
    {

    }

    driverpacman::~driverpacman()
    {

    }

    string driverpacman::drivername()
    {
	return "pacman";
    }

    bool driverpacman::hasdriverROMs()
    {
	return true;
    }

    bool driverpacman::drvinit()
    {
	loadROM("pacman.6e", 0x0000, 0x1000, inter.gamerom);
	loadROM("pacman.6f", 0x1000, 0x1000, inter.gamerom);
	loadROM("pacman.6h", 0x2000, 0x1000, inter.gamerom);
	loadROM("pacman.6j", 0x3000, 0x1000, inter.gamerom);
	inter.init();
	resize(640, 480, 1);
	return isallfilesloaded();
    }

    void driverpacman::drvshutdown()
    {
	inter.shutdown();
    }
  
    void driverpacman::drvrun()
    {
	inter.run();
	fillrect(0, 0, 640, 480, red());
    }

    void driverpacman::drvcoin(bool pressed)
    {
	if (pressed)
	{
	    cout << "Coin button pressed" << endl;
	}
    }

    void driverpacman::drvstartp1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Start button pressed" << endl;
	}
    }

    void driverpacman::drvleftp1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Left button pressed" << endl;
	}
    }

    void driverpacman::drvrightp1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Right button pressed" << endl;
	}
    }

    void driverpacman::drvfirep1(bool pressed)
    {
	if (pressed)
	{
	    cout << "Fire button pressed" << endl;
	}
    }
};