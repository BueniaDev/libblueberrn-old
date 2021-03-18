#ifndef BERRN_MAIN
#define BERRN_MAIN

#include <functional>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "berrn/drivers/drivers.h"
using namespace std;

namespace berrn
{
    struct berrninitprog
    {
        bool getoptions(int argc, char* argv[]);
	bool preinit();
        bool driverloadedcmd = false;
	string drivername = "";

	void init();
	void deinit();
	void run();

	berrndriver *drv = NULL;
	berrnfrontend front;

	DriverList drivers;

	void connectdriver(berrndriver *connect)
	{
	    drv = connect;
	}

	void setcallbacks(initfunc icb, deinitfunc dcb, drawfunc drcb)
	{
	    if (drv != NULL)
	    {
	        drv->setcallbacks(icb, dcb, drcb);
	    }
	    else
	    {
		cout << "Object null" << endl;
		exit(1);
	    }
	}

	void coin(bool pressed)
	{
	    drv->coin(pressed);
	}

	void keyleft(bool pressed)
	{
	    drv->keyleft(pressed);
	}

	void keyright(bool pressed)
	{
	    drv->keyright(pressed);
	}

	void keystart(bool pressed)
	{
	    drv->keystart(pressed);
	}

	void keyaction(bool pressed)
	{
	    drv->keyaction(pressed);
	}
    };
};

#endif // BERRN_MAIN
