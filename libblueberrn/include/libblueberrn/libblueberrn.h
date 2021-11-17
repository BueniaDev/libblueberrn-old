#ifndef LIBBLUEBERRN_H
#define LIBBLUEBERRN_H

#include <driver.h>
#include <driverlist.h>
#include <iostream>
#include <vector>
using namespace berrn;
using namespace std;

namespace berrn
{
    class BlueberrnCore
    {
	public:
	    BlueberrnCore();
	    ~BlueberrnCore();

	    DriverList list;
	    berrndriver *driver;
      
	    BlueberrnFrontend *front = NULL;
      
	    void setfrontend(BlueberrnFrontend *cb);
      
	    bool initdriver(string name);

	    bool getoptions(int argc, char* argv[]);
      
	    vector<string> getdrvnames();
      
	    bool init();
	    void shutdown();
      
	    bool nocmdarguments();
      
	    int cmdargc = 0;
      
	    bool startdriver(bool isready = true);
	    void stopdriver(bool isready = true);
	    void rundriver();

	    void keychanged(BerrnInput key, bool is_pressed);
	    void keypressed(BerrnInput key);
	    void keyreleased(BerrnInput key);
      
	    void runapp();
    };
}

#endif // LIBBLUEBERRN_H