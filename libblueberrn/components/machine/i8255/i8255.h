/*
    This file is part of libblueberrn.
    Copyright (C) 2021 BueniaDev.

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

#ifndef LIBBLUEBERRN_I8255_PPI_H
#define LIBBLUEBERRN_I8255_PPI_H

#include <cstdint>
#include <iostream>
using namespace std;


// Single-header Intel I8255 emulator (WIP)
namespace i8255
{
    class i8255ppi
    {
	public:
	    i8255ppi()
	    {

	    }

	    ~i8255ppi()
	    {

	    }

	    void write(int addr, uint8_t data)
	    {
		switch (addr & 0x3)
		{
		    case 0:
		    {
			cout << "Writing value of " << hex << int(data) << " to I8255 port A" << endl;
		    }
		    break;
		    case 1:
		    {
			cout << "Writing value of " << hex << int(data) << " to I8255 port B" << endl;
		    }
		    break;
		    case 2:
		    {
			cout << "Writing value of " << hex << int(data) << " to I8255 port C" << endl;
		    }
		    break;
		    case 3:
		    {
			if (testbit(data, 7))
			{
			    cout << "Setting I8255 control word to " << hex << int(data) << endl;
			}
			else
			{
			    int bit = ((data >> 1) & 0x7);
			    bool is_set = testbit(data, 0);

			    string bit_status = (is_set) ? "Setting" : "Resetting";
			    cout << bit_status << " bit " << dec << bit << " of I8255 port C" << endl;
			}
		    }
		    break;
		}
	    }

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
	    }
    };
};


#endif // LIBBLUEBERRN_I8255_PPI_H