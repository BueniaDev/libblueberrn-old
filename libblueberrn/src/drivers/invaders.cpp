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

#include <invaders.h>
using namespace berrn;
using namespace std;
using namespace std::placeholders;

namespace berrn
{
    InvadersInterface::InvadersInterface()
    {

    }

    InvadersInterface::~InvadersInterface()
    {

    }

    uint8_t InvadersInterface::readByte(uint16_t addr)
    {
	uint8_t temp = 0;

	addr &= 0x3FFF;
	if (addr < 0x2000)
	{
	    temp = gamerom[addr];   
	}
	else if (addr < 0x2400)
	{
	    temp = workram[(addr - 0x2000)];
	}
	else if (addr < 0x4000)
	{
	    temp = videoram[(addr - 0x2400)];
	}
	else
	{
	    temp = 0xFF;
	}

	return temp;
    }

    void InvadersInterface::writeByte(uint16_t addr, uint8_t val)
    {
	addr &= 0x3FFF;
	if (addr < 0x2000)
	{
	    return;
	}
	else if (addr < 0x2400)
	{
	    workram[(addr - 0x2000)] = val;
	}
	else if (addr < 0x4000)
	{
	    videoram[(addr - 0x2400)] = val;
	}
	else
	{
	    return;
	}
    }

    uint8_t InvadersInterface::portIn(uint8_t port)
    {
	uint8_t temp = 0;

	switch (port)
	{
	    case 1: temp = port1; break; // TODO: Rest of user input
	    case 2: temp = 0; break; // TODO: Rest of user input
	    case 3: temp = shifter.readshiftresult(); break;
	    default:
	    {
		cout << "Reading from port of " << hex << (int)(port) << endl;
		exit(0);
	    }
	    break;
	}

	return temp;
    }

    void InvadersInterface::portOut(uint8_t port, uint8_t val)
    {
	switch (port)
	{
	    case 2: shifter.setshiftoffs(val); break;
	    case 3:
	    {
		playSound(val, false);
	    }
	    break;
	    case 4: shifter.fillshiftreg(val); break;
	    case 5:
	    {
		playSound(val, true);
	    }
	    break;
	    case 6: /* debugPort(val); */ break;
	    default:
	    {
		cout << "Writing to port of " << hex << (int)(port) << ", value of " << hex << (int)(val) << endl;
		exit(0);
	    }
	    break;
	}
    }

    void InvadersInterface::loadSound(int id)
    {
	soundIDs.push_back(id);
    }

    void InvadersInterface::playSound(uint8_t val, bool bank)
    {
	if (soundfunc)
	{
	    soundfunc(val, bank);
	}
    }

    void InvadersInterface::setsoundfunc(invsoundfunc cb)
    {
	soundfunc = cb;
    }

    void InvadersInterface::init()
    {
	// Bit 3 of port 1 is always set
	port1 = 8;
	workram.resize(0x400, 0);
	videoram.resize(0x1C00, 0);
	core.setinterface(this);
	core.init();
    }

    void InvadersInterface::shutdown()
    {
	// Reset port 1
	port1 = 0;
	core.shutdown();
	soundIDs.clear();
	videoram.clear();
	workram.clear();
	gamerom.clear();
    }

    void InvadersInterface::reset()
    {
	core.reset();
	fill(videoram.begin(), videoram.end(), 0);
	fill(workram.begin(), workram.end(), 0);
    }

    void InvadersInterface::run()
    {
	for (int i = 0; i < 2; i++)
	{
	    runHalfFrame();
	    uint8_t interrupt_instr = (i != 0) ? interrupt1 : interrupt0;
	    core.setinterrupt(interrupt_instr);
	}
    }

    void InvadersInterface::coin(bool pressed)
    {
	changePort1Bit(0, pressed);
    }

    void InvadersInterface::startp1(bool pressed)
    {
	changePort1Bit(2, pressed);
    }

    void InvadersInterface::leftp1(bool pressed)
    {
	changePort1Bit(5, pressed);
    }

    void InvadersInterface::rightp1(bool pressed)
    {
	changePort1Bit(6, pressed);
    }

    void InvadersInterface::firep1(bool pressed)
    {
	changePort1Bit(4, pressed);
    }

    void InvadersInterface::updatepixels(vector<berrnRGBA> &buffer)
    {
	if (static_cast<int>(buffer.size()) != (width * height))	
	{
	    return;
	}

	for (int i = 0; i < (width * (height / 8)); i++)
	{
	    int y = ((i * 8) / 256);
	    int basex = ((i * 8) % 256);

	    uint8_t currentbyte = videoram[i];

	    for (int bit = 0; bit < 8; bit++)
	    {
		int pixelx = basex + bit;
		int pixely = y;
		bool ispixellit = ((currentbyte >> bit) & 0x1);

		int tempx = pixelx;
		pixelx = pixely;
		pixely = (-tempx + width - 1);

		int index = (pixelx + (pixely * 224));

		buffer[index] = (ispixellit) ? white() : black();
	    }
	} 
    }

    void InvadersInterface::runHalfFrame()
    {
	while (totalcycles < cycles_per_half_frame)
	{
	    totalcycles += core.runinstruction();
	}

	totalcycles = 0;
    }

    void InvadersInterface::changePort1Bit(int bit, bool pressed)
    {
	if (pressed)
	{
	    port1 |= (1 << bit);
	}
	else
	{
	    port1 &= ~(1 << bit);
	}
    }

    void InvadersInterface::debugPort(uint8_t val)
    {
	char debugchar;
	uint8_t writtenval = val;
	writtenval += 65; // First letter of the ASCII alphabet

	if (writtenval < 65 || writtenval > 90)
	{
	    debugchar = '\n';
	}
	else
	{
	    debugchar = static_cast<char>(writtenval);
	}

	cout.put(debugchar);
    }

    driverinvaders::driverinvaders()
    {

    }

    driverinvaders::~driverinvaders()
    {

    }

    string driverinvaders::drivername()
    {
	return "invaders";
    }

    bool driverinvaders::hasdriverROMs()
    {
	return true;
    }

    bool driverinvaders::drvinit()
    {
	prev_port3 = bitset<4>(0);
	prev_port5 = bitset<5>(0);
	inter.init();
	inter.setsoundfunc(bind(&driverinvaders::invadersSound, this, _1, _2));
	loadROM("invaders.h", 0x0000, 0x0800, inter.gamerom);
	loadROM("invaders.g", 0x0800, 0x0800, inter.gamerom);
	loadROM("invaders.f", 0x1000, 0x0800, inter.gamerom);
	loadROM("invaders.e", 0x1800, 0x0800, inter.gamerom);

	loadInvSound("8.wav"); // UFO sound
	loadInvSound("1.wav"); // shoot sound
	loadInvSound("2.wav"); // player die
	loadInvSound("3.wav"); // alien die
	loadInvSound("4.wav"); // alien move 1
	loadInvSound("5.wav"); // alien move 2
	loadInvSound("6.wav"); // alien move 3
	loadInvSound("7.wav"); // alien move 4
	loadInvSound("10.wav"); // UFO hit

	for (int i = 0; i < 9; i++)
	{
	    setSoundVol(i, 0.25);
	}

	framebuffer.resize((256 * 224), black());
	resize(224, 256, 2);
    
	return isallfilesloaded();
    }

    void driverinvaders::loadInvSound(string filename)
    {
	soundIDs.push_back(loadWAV(filename));
    }

    void driverinvaders::drvshutdown()
    {
	inter.shutdown();
    }
  
    void driverinvaders::drvrun()
    {
	inter.run();
	inter.updatepixels(framebuffer);
	filltexrect(0, 0, 224, 256, framebuffer);
	outputSamples();
    }

    void driverinvaders::invadersSound(uint8_t val, bool bank)
    {
	if (!bank)
	{
	    bitset<4> port3_val = bitset<4>(val);

	    if (port3_val.test(0) && !prev_port3.test(0))
	    {
		playSound(soundIDs[0]);
	    }

	    if (port3_val.test(1) && !prev_port3.test(1))
	    {
		playSound(soundIDs[1]);
	    }

	    if (port3_val.test(2) && !prev_port3.test(2))
	    {
		playSound(soundIDs[2]);
	    }

	    if (port3_val.test(3) && !prev_port3.test(3))
	    {
		playSound(soundIDs[3]);
	    }

	    prev_port3 = port3_val;
	}
	else
	{
	    bitset<5> port5_val = bitset<5>(val);

	    if (port5_val.test(0) && !prev_port5.test(0))
	    {
		playSound(soundIDs[4]);
	    }

	    if (port5_val.test(1) && !prev_port5.test(1))
	    {
		playSound(soundIDs[5]);
	    }

	    if (port5_val.test(2) && !prev_port5.test(2))
	    {
		playSound(soundIDs[6]);
	    }

	    if (port5_val.test(3) && !prev_port5.test(3))
	    {
		playSound(soundIDs[7]);
	    }

	    if (port5_val.test(4) && !prev_port5.test(4))
	    {
		playSound(soundIDs[8]);
	    }

	    prev_port5 = port5_val;
	}
    }

    void driverinvaders::drvcoin(bool pressed)
    {
	inter.coin(pressed);
    }

    void driverinvaders::drvstartp1(bool pressed)
    {
	inter.startp1(pressed);
    }

    void driverinvaders::drvleftp1(bool pressed)
    {
	inter.leftp1(pressed);
    }

    void driverinvaders::drvrightp1(bool pressed)
    {
	inter.rightp1(pressed);
    }

    void driverinvaders::drvfirep1(bool pressed)
    {
	inter.firep1(pressed);
    }
};