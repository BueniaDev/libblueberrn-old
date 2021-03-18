#ifndef BERRN_DRIVERS
#define BERRN_DRIVERS

#include <functional>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

namespace berrn
{
    struct RGB
    {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
    };

    using initfunc = function<void(int, int)>;
    using deinitfunc = function<void()>;
    using drawfunc = function<void(RGB*, int, int, int)>;

    struct berrndrivername
    {
	string name;
	string fullname;
	string parent;
	string year;
	string company;
    };

    struct berrnutil
    {
	bool loadROM(string filename, vector<uint8_t> &mem, int start);
    };

    struct berrnfrontend
    {
	initfunc initfront;
	deinitfunc deinitfront;
	drawfunc drawfront;

	inline void setinitcallbacks(initfunc icb, deinitfunc dcb)
	{
	    initfront = icb;
	    deinitfront = dcb;
	}

	inline void setdrawcallback(drawfunc cb)
	{
	    drawfront = cb;
	}
    };

    class berrndriver
    {
	public:
	berrndriver();
	~berrndriver();
	berrndrivername name;
	berrnfrontend front;

	virtual void setcallbacks(initfunc icb, deinitfunc dcb, drawfunc drcb) = 0;

	virtual void init() = 0;
	virtual void deinit() = 0;
	virtual void run() = 0;

	virtual void coin(bool pressed) = 0;
	virtual void keyleft(bool pressed) = 0;
	virtual void keyright(bool pressed) = 0;
	virtual void keystart(bool pressed) = 0;
	virtual void keyaction(bool pressed) = 0;

	virtual void initdriver(int width, int height) = 0;
	virtual void deinitdriver() = 0;
	virtual void drawpixels(RGB* buffer, int width, int height, int scale) = 0;
    };
};

#endif // BERRN_DRIVERS
