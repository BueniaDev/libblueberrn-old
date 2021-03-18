#ifndef GALAXIAN_DRIVER
#define GALAXIAN_DRIVER

#include "../../../../../../src/core/cpu/z80/z80.h"
#include "../../../berrn.h"
using namespace zilogz80;
using namespace berrn;
using namespace std;

namespace berrn
{
    class galaxian: public berrndriver
    {
	public:
	galaxian();
	~galaxian();

	berrndrivername name;
	berrnfrontend front;   

	void coin(bool pressed)
	{

	}  

	void setcallbacks(initfunc icb, deinitfunc dcb, drawfunc drcb)
	{
	    front.setinitcallbacks(icb, dcb);
	    front.setdrawcallback(drcb);
	}

	uint8_t readmem(uint16_t address);
        void writemem(uint16_t address, uint8_t value);
	void driverinfo();
        void init();
	void deinit();
	void run();
	void updateframebuffer();
        z80 galaxcpu;
	int overspentcycles = 0;

	void initdriver(int width, int height)
	{
	    front.initfront(width, height);
	}

	void deinitdriver()
	{
	    front.deinitfront();
	}

	void drawpixels(RGB *buffer, int width, int height, int scale)
	{
	    front.drawfront(buffer, width, height, scale);
	}

	void keyleft(bool pressed)
	{

	}

	void keyright(bool pressed)
	{

	}

	void keystart(bool pressed)
	{

	}

	void keyaction(bool pressed)
	{

	}

	berrnutil util;

	vector<uint8_t> rom;
    };
};

#endif // GALAXIAN_DRIVER
