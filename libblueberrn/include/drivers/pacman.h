#ifndef BERRN_PACMAN
#define BERRN_PACMAN

#include <libblueberrn_api.h>
#include <driver.h>
#include <iostream>
#include <string>
using namespace berrn;
using namespace std;

namespace berrn
{
    class LIBBLUEBERRN_API driverpacman : public berrndriver
    {
	public:
	    driverpacman();
	    ~driverpacman();

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


#endif // BERRN_PACMAN