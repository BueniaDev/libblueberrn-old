#include "../../include/libblueberrn/libblueberrn.h"
using namespace berrn;
using namespace std;

namespace berrn
{
    BlueberrnCore::BlueberrnCore()
    {

    }

    BlueberrnCore::~BlueberrnCore()
    {

    }

    bool BlueberrnCore::getoptions(int argc, char **argv)
    {
	list.init();
	cmdargc = argc;
  	
	if (cmdargc < 2)
	{
	    return true;
	}

	return initdriver(argv[1]);
    }
  
    bool BlueberrnCore::initdriver(string name)
    {
	driver = searchdriver(name);

	if (driver == NULL)
	{
	    cout << "Error: could not find driver" << endl;
	    return false;
	}
    
	return true;
    }
  
    vector<string> BlueberrnCore::getdrvnames()
    {
	return getdrivernames();
    }

    bool BlueberrnCore::startdriver(bool isready)
    {
	if (!isready)
	{
	    return true;
	}
  
	if (driver != NULL)
	{
	    cout << "Initializing driver..." << endl;
	    driver->setfrontend(front);
	    return driver->startdriver();
	}
	else
	{
	    return true;
	}
    }

    void BlueberrnCore::stopdriver(bool isready)
    {
	if (!isready)
	{
	    return;
	}
  
	if (driver != NULL)
	{
	    driver->stopdriver();
	    driver = NULL;
	}
    }

    void BlueberrnCore::rundriver()
    {
	if (driver == NULL)
	{
	    return;
	}

	driver->rundriver();
	driver->drawpixels();
    }

    void BlueberrnCore::keypressed(BerrnInput key)
    {
	if (driver != NULL)
	{
	    driver->keypressed(key);
	}
    }

    void BlueberrnCore::keyreleased(BerrnInput key)
    {
	if (driver != NULL)
	{
	    driver->keyreleased(key);
	}
    }
  
    void BlueberrnCore::setfrontend(BlueberrnFrontend *cb)
    {
	front = cb;
    }
  
    bool BlueberrnCore::init()
    {
	if (front != NULL)
	{
	    return front->init();
	}
	else
	{
	    return true;
	}
    }
  
    void BlueberrnCore::shutdown()
    {
	if (front != NULL)
	{
	    front->shutdown();
	}
    }
  
    void BlueberrnCore::runapp()
    {
	if (front != NULL)
	{
	    front->runapp();
	}
    }
  
    bool BlueberrnCore::nocmdarguments()
    {
	return (cmdargc < 2);
    }
};