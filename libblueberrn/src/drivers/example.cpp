#include "example.h"
using namespace berrn;

namespace berrn
{
    driverexample::driverexample()
    {

    }

    driverexample::~driverexample()
    {

    }

    string driverexample::drivername()
    {
	return "example";
    }

    bool driverexample::hasdriverROMs()
    {
	return false;
    }

    bool driverexample::drvinit()
    {
	resize(640, 480, 1);
	return true;
    }

    void driverexample::drvshutdown()
    {
	return;
    }
  
    void driverexample::drvrun()
    {
	fillrect(0, 0, 640, 480, red());
	fillrect(50, 50, 100, 100, green());
    }

    void driverexample::drvcoin(bool pressed)
    {
	return;
    }

    void driverexample::drvstartp1(bool pressed)
    {
	return;
    }

    void driverexample::drvleftp1(bool pressed)
    {
	return;
    }

    void driverexample::drvrightp1(bool pressed)
    {
	return;
    }

    void driverexample::drvfirep1(bool pressed)
    {
	return;
    }
};