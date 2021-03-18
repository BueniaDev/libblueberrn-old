#include "../../../../../include/libblueberrn/berrn/drivers/example/example.h"
using namespace std;
using namespace example;
using namespace berrn;

namespace example
{
    void exampledrv::berrninit()
    {
	initdriver(640, 480);
    }

    void exampledrv::berrndeinit()
    {
	deinitdriver();
    }
};
