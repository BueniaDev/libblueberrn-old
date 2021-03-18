#include "../../../../../../include/libblueberrn/berrn/drivers/namco/galaxian/galaxian.h"
using namespace std;
using namespace std::placeholders;
using namespace berrn;

namespace berrn
{
    galaxian::galaxian()
    {
	driverinfo();
    }

    galaxian::~galaxian()
    {

    }

    void galaxian::init()
    {
	rom.resize(0x4000, 0);
	
	if (!util.loadROM("galmidw.u", rom, 0))
	{
	    exit(1);
	}

	if (!util.loadROM("galmidw.v", rom, 0x800))
	{
	    exit(1);
	}

	if (!util.loadROM("galmidw.w", rom, 0x1000))
	{
	    exit(1);
	}

	if (!util.loadROM("galmidw.y", rom, 0x1800))
	{
	    exit(1);
	}

	if (!util.loadROM("7l", rom, 0x2000))
	{
	    exit(1);
	}

        auto rb = bind(&galaxian::readmem, this, _1);
        auto wb = bind(&galaxian::writemem, this, _1, _2);
        galaxcpu.setmemcallback(rb, wb);

	galaxcpu.reset();

	initdriver((224 * 2), (256 * 2));
    }

    void galaxian::driverinfo()
    {
	name.name = "galaxian";
	name.fullname = "Namco Galaxian (set 1)";
	name.year = "1979";
	name.company = "Namco";
    }

    uint8_t galaxian::readmem(uint16_t address)
    {
	uint8_t temp;

	if (address < 0x4000)
	{
	    temp = rom[address];
	}
	else
	{
	    temp = 0xFF;
	}

	return temp;
    }

    void galaxian::writemem(uint16_t address, uint8_t value)
    {
	if (address < 0x4000)
	{
	    return;
	}
	else
	{
	    return;
	}
    }

    void galaxian::deinit()
    {
	deinitdriver();
    }

    void galaxian::run()
    {
	overspentcycles = galaxcpu.runfor(51200 + overspentcycles);
    }
};
