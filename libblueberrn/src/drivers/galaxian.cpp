#include "galaxian.h"
using namespace berrn;

namespace berrn
{
    drivergalaxian::drivergalaxian()
    {

    }

    drivergalaxian::~drivergalaxian()
    {

    }

    string drivergalaxian::drivername()
    {
	return "galaxian";
    }

    bool drivergalaxian::hasdriverROMs()
    {
	return false;
    }

    bool drivergalaxian::drvinit()
    {
	resize(640, 480, 1);
	return true;
    }

    void drivergalaxian::drvshutdown()
    {
	return;
    }
  
    void drivergalaxian::drvrun()
    {
	fillrect(0, 0, 640, 480, red());
    }

    void drivergalaxian::drvcoin(bool pressed)
    {
	return;
    }

    void drivergalaxian::drvstartp1(bool pressed)
    {
	return;
    }

    void drivergalaxian::drvleftp1(bool pressed)
    {
	return;
    }

    void drivergalaxian::drvrightp1(bool pressed)
    {
	return;
    }

    void drivergalaxian::drvfirep1(bool pressed)
    {
	return;
    }
};