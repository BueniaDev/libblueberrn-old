#include "../../../../../include/libblueberrn/berrn/drivers/midway/invaders.h"
using namespace std;
using namespace std::placeholders;
using namespace berrn;

namespace berrn
{
    invaders::invaders()
    {
	driverinfo();
    }

    invaders::~invaders()
    {

    }

    void invaders::init()
    {
	rom.resize(0x2000, 0);
	wram.resize(0x400, 0);
	vram.resize(0x1C00, 0);
	
	if (!util.loadROM("invaders.h", rom, 0))
	{
	    exit(1);
	}

	if (!util.loadROM("invaders.g", rom, 0x800))
	{
	    exit(1);
	}

	if (!util.loadROM("invaders.f", rom, 0x1000))
	{
	    exit(1);
	}

	if (!util.loadROM("invaders.e", rom, 0x1800))
	{
	    exit(1);
	}

        auto rb = bind(&invaders::readmem, this, _1);
        auto wb = bind(&invaders::writemem, this, _1, _2);
	auto rp = bind(&invaders::readport, this, _1);
	auto wp = bind(&invaders::writeport, this, _1, _2);
        invcpu.setmemcallback(rb, wb);
	invcpu.setportcallback(rp, wp);

	invcpu.reset();

	initdriver((224 * 2), (256 * 2));
    }

    void invaders::driverinfo()
    {
	name.name = "invaders";
	name.fullname = "Space Invaders";
	name.year = "1978";
	name.company = "Taito";
    }

    uint8_t invaders::readmem(uint16_t address)
    {
	uint8_t temp;

	address = (address % 0x4000);
	
	if (address < 0x2000)
	{
	    temp = rom[address];
	}
	else if (address < 0x2400)
	{
	    temp = wram[address - 0x2000];
	}
	else if (address < 0x4000)
	{
	    temp = vram[address - 0x2400];
	}
	else
	{
	    temp = 0xFF;
	}

	return temp;
    }

    void invaders::writemem(uint16_t address, uint8_t value)
    {
	address = (address % 0x4000);	

	if (address < 0x2000)
	{
	    return;
	}
	else if (address < 0x2400)
	{
	    wram[address - 0x2000] = value;
	}
	else if (address < 0x4000)
	{
	    vram[address - 0x2400] = value;
	}
	else
	{
	    return;
	}
    }

    uint8_t invaders::readport(uint8_t address)
    {
	uint8_t temp = 0;

	switch (address)
	{
	    case 1: temp = port1; break;
	    case 2: temp = 0; break;
	    case 3: temp = shift.getresult(); break;
	    default: unrecognizedportread(address); break;
	}

	return temp;
    }

    void invaders::writeport(uint8_t address, uint8_t value)
    {
	switch (address)
	{
	    case 2: shift.setshiftoffset(value); break;
	    case 3: break;
	    case 4: shift.fillshiftreg(value); break;
	    case 5: break;
	    case 6: /* cout << getdebugchar(value); */ break;
	    default: unrecognizedportwrite(address, value); break;
	}
    }

    char invaders::getdebugchar(uint8_t value)
    {
	uint8_t temp = value;
	temp += 65; // First letter of ASCII alphabet

	if (temp < 65 || temp > 90)
	{
	    return '\n';
	}

	return (char)(int)(temp);
    }

    void invaders::unrecognizedportread(uint8_t address)
    {
	cout << "Unrecognized port read into port number " << (int)(address) << endl;
	exit(1);
    }

    void invaders::unrecognizedportwrite(uint8_t address, uint8_t value)
    {
	cout << "Unrecognized port write of value " << (int)(value) << " to port number " << (int)(address) << endl;
	exit(1);
    }

    void invaders::deinit()
    {
	invcpu.shutdown();
	deinitdriver();
    }

    void invaders::run()
    {	
	overspentcycles = invcpu.runfor(16666 + overspentcycles);
	invcpu.generateinterrupt(1);
	updateframebuffer();
	overspentcycles = invcpu.runfor(16666 + overspentcycles);
	invcpu.generateinterrupt(2);
	updateframebuffer();

	drawpixels(framebuffer, 224, 256, 2);
    }

    void invaders::coin(bool pressed)
    {
	port1 = (pressed) ? (port1 | 0x01) : (port1 & ~0x01);
    }

    void invaders::keyleft(bool pressed)
    {
	port1 = (pressed) ? (port1 | 0x20) : (port1 & ~0x20);
    }

    void invaders::keyright(bool pressed)
    {
	port1 = (pressed) ? (port1 | 0x40) : (port1 & ~0x40);
    }

    void invaders::keystart(bool pressed)
    {
	port1 = (pressed) ? (port1 | 0x04) : (port1 & ~0x04);
    }

    void invaders::keyaction(bool pressed)
    {
	port1 = (pressed) ? (port1 | 0x10) : (port1 & ~0x10);
    }

    void invaders::updateframebuffer()
    {
	for (int i = 0; i < (256 * (224 / 8)); i++)
	{
	    int y = ((i * 8) / 256);
	    int basex = ((i * 8) % 256);
	    uint8_t currentbyte = readmem(0x2400 + i);

	    for (int bit = 0; bit < 8; bit++)
	    {
		int pixelx = basex + bit;
		int pixely = y;
		bool ispixellit = ((currentbyte >> bit) & 0x01);

		int color = (ispixellit) ? 255 : 0;
		

		int tempx = pixelx;
		pixelx = pixely;
		pixely = (-tempx + 256 - 1);

		int index = (pixelx + (pixely * 224));
		framebuffer[index].red = color;
		framebuffer[index].green = color;
		framebuffer[index].blue = color;
	    }
	}
    }
};
