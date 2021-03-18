#ifndef BERRN_EXAMPLE
#define BERRN_EXAMPLE

#include <libblueberrn_api.h>
#include <driver.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API driverexample : public berrndriver
    {
	public:
	    driverexample();
	    ~driverexample();

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
    };
};


#endif // BERRN_EXAMPLE