#ifndef EXAMPLE_DRIVER
#define EXAMPLE_DRIVER

#include "../../berrn.h"
using namespace berrn;

namespace example
{
    class exampledrv : public berrndriver
    {
	exampledrv();
	~exampledrv(); 

	berrndrivername name;
	berrnfrontend front;

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

	void berrninit();
	void berrndeinit();
    };
    
};

#endif // EXAMPLE_DRIVER
