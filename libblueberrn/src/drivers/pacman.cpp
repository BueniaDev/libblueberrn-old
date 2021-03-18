#include "pacman.h"
using namespace berrn;

namespace berrn
{
    driverpacman::driverpacman()
    {

    }

    driverpacman::~driverpacman()
    {

    }

    string driverpacman::drivername()
    {
	return "pacman";
    }

    bool driverpacman::hasdriverROMs()
    {
	return false;
    }

    bool driverpacman::drvinit()
    {
	resize(640, 480, 1);
	return true;
    }

    void driverpacman::drvshutdown()
    {
	return;
    }
  
    void driverpacman::drvrun()
    {
	fillrect(0, 0, 640, 480, red());
    }

    void driverpacman::drvcoin(bool pressed)
    {
	return;
    }

    void driverpacman::drvstartp1(bool pressed)
    {
	return;
    }

    void driverpacman::drvleftp1(bool pressed)
    {
	return;
    }

    void driverpacman::drvrightp1(bool pressed)
    {
	return;
    }

    void driverpacman::drvfirep1(bool pressed)
    {
	return;
    }
};