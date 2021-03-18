#ifndef BERRN_DRIVER_LIST
#define BERRN_DRIVER_LIST

#include <functional>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "midway/invaders.h"
#include "namco/galaxian/galaxian.h"
using namespace std;
using namespace berrn;

namespace berrn
{
class DriverList
{
    public:
	DriverList();
	~DriverList();
    invaders invdriver;
    galaxian galaxdriver;
};
};


#endif // BERRN_DRIVER_LIST
