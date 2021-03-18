#include "../../../include/libblueberrn/libblueberrn.h"
using namespace std;

namespace berrn
{
    DriverList::DriverList()
    {

    }

    DriverList::~DriverList()
    {

    }

    bool berrninitprog::preinit()
    {
	if (drivername == "invaders")
	{
	    cout << "Driver name: Space Invaders (1978)" << endl;
	    connectdriver(&drivers.invdriver);
	    return true;
	}
	else if (drivername == "galaxian")
	{
	    cout << "Driver name: Namco Galaxian (1979, set 1)" << endl; 
	    connectdriver(&drivers.galaxdriver);
	    return true;
	}
	else
	{
	    cout << "Invalid driver name" << endl;
	    return false;
	}

	return false;
    }
};
