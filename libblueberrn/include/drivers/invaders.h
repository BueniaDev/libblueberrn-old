#ifndef BERRN_INVADERS
#define BERRN_INVADERS

#include <libblueberrn_api.h>
#include <driver.h>
#include <cpu/intel8080.h>
#include <machine/mb14241.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API InvadersInterface : public Bee8080Interface
    {
	public:
	    InvadersInterface();
	    ~InvadersInterface();

	    void init();
	    void shutdown();
	    void reset();
	    void run();
	    void updatepixels(vector<berrnRGBA> &buffer);

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t val);
	    uint8_t portIn(uint8_t port);
	    void portOut(uint8_t port, uint8_t val);

	    void coin(bool pressed);
	    void startp1(bool pressed);
	    void leftp1(bool pressed);
	    void rightp1(bool pressed);
	    void firep1(bool pressed);

	    vector<uint8_t> gamerom;

	    vector<uint8_t> workram;
	    vector<uint8_t> videoram;

	private:
	    const int cycles_per_half_frame = ((2 * 1000000) / 60) / 2; // 2 MHz at 60 fps, divided by two

	    const uint8_t interrupt0 = 0xCF; // RST 1 at the start of VBlank
	    const uint8_t interrupt1 = 0xD7; // RST 2 at the end of VBlank

	    const int width = 256;
	    const int height = 224;

	    int runHalfFrame();

	    void changePort1Bit(int bit, bool pressed);

	    void debugPort(uint8_t val);

	    uint8_t port1 = 0;

	    Bee8080 core;
	    mb14241shifter shifter;
    };

    class LIBBLUEBERRN_API driverinvaders : public berrndriver
    {
	public:
	    driverinvaders();
	    ~driverinvaders();

	    string drivername();
	    bool hasdriverROMs();

	    bool drvinit();
	    void drvshutdown();
	    void drvrun();

	    void drvcoin(bool pressed);
	    void drvstartp1(bool pressed);
	    void drvleftp1(bool pressed);
	    void drvrightp1(bool pressed);
	    void drvfirep1(bool pressed);

	    InvadersInterface inter;

	    vector<berrnRGBA> framebuffer;
    };
};

#endif // BERRN_INVADERS