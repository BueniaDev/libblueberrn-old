#ifndef INVADERS_DRIVER
#define INVADERS_DRIVER

#include "../../../../../src/core/cpu/8080/8080.h"
#include "../../../machine/mb14241.h"
#include "../../berrn.h"
using namespace intel8080;
using namespace mb14241;
using namespace berrn;
using namespace std;

namespace berrn
{
    class invaders : public berrndriver
    {
	public:
	invaders();
	~invaders();

	berrndrivername name;
	berrnfrontend front; 

	void setcallbacks(initfunc icb, deinitfunc dcb, drawfunc drcb)
	{
	    front.setinitcallbacks(icb, dcb);
	    front.setdrawcallback(drcb);
	}

	uint8_t readmem(uint16_t address);
        void writemem(uint16_t address, uint8_t value);
	uint8_t readport(uint8_t address);
	void writeport(uint8_t address, uint8_t value);
	char getdebugchar(uint8_t value);
	void unrecognizedportread(uint8_t address);
	void unrecognizedportwrite(uint8_t address, uint8_t value);
	void driverinfo();
        void init();
	void deinit();
	void run();
	void coin(bool pressed);
	void keyleft(bool pressed);
	void keyright(bool pressed);
	void keystart(bool pressed);
	void keyaction(bool pressed);

	void initdriver(int width, int height)
	{
	    front.initfront(width, height);
	}

	void deinitdriver()
	{
	    front.deinitfront();
	}

	void drawpixels(RGB* buffer, int width, int height, int scale)
	{
	    front.drawfront(buffer, width, height, scale);
	}
	
	void updateframebuffer();
        i8080 invcpu;
	mb14241shifter shift;
	berrnutil util;

	uint8_t port1;

	vector<uint8_t> rom;
	vector<uint8_t> wram;
	vector<uint8_t> vram;

	RGB framebuffer[256 * 224];

	int overspentcycles = 0;
    };
};

#endif // INVADERS_DRIVER
