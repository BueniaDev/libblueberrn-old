#include "../../include/libblueberrn/libblueberrn.h"

namespace berrn
{

    bool berrninitprog::getoptions(int argc, char* argv[])
    {
	if (argc < 2)
	{
	    cout << "No driver name specified in arguments" << endl;
	    return false;
	}
	else
	{
	    drivername = argv[1];
	    driverloadedcmd = preinit();
	    return driverloadedcmd;
	}
    }

    void berrninitprog::init()
    {
	drv->init();
    }

    void berrninitprog::deinit()
    {
	drv->deinit();
    }

    void berrninitprog::run()
    {
	drv->run();
    }
};
