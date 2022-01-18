/*
    This file is part of libblueberrn.
    Copyright (C) 2022 BueniaDev.

    libblueberrn is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libblueberrn is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libblueberrn.  If not, see <https://www.gnu.org/licenses/>.
*/

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

	    berrndriver *getDriver()
	    {
		return driver;
	    }
      
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