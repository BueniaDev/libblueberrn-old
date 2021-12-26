#ifndef BERRN_DRIVERLIST
#define BERRN_DRIVERLIST

#include <berrndrivers.h>
#include <iostream>
#include <libblueberrn_api.h>
using namespace std;
using namespace berrn;

namespace berrn
{
    struct DriverList
    {
	void init()
	{
	    DRIVER(example);
	    DRIVER(invaders);
	    DRIVER(galaxian);
	    DRIVER(pacman);
	    DRIVER(galaga);
	    DRIVER(wboy2u);
	    DRIVER(rastan);
	}
    };
};

#endif // BERRN_DRIVERLIST