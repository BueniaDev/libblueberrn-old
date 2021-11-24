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

	    void init()
	    {
		set_mode(0x9B);
		cout << "I8255::Initialized" << endl;
	    }

	    void shutdown()
	    {
		cout << "I8255::Shutting down..." << endl;
	    }

	    void write(int addr, uint8_t data)
	    {
		switch ((addr & 0x3))
		{
		    case 1: write_port_b(data); break;
		    case 3:
		    {
			if (testbit(data, 7))
			{
			    set_mode(data);
			}
			else
			{
			    int bit_to_change = ((data >> 1) & 0x7);
			    bool state = testbit(data, 0);
			    string port_state = (state ? "Setting" : "Resetting");
			    cout << port_state << " bit " << dec << bit_to_change << " of I8255 port C" << endl;
			    exit(0);
			}
		    }
		    break;
		    default:
		    {
			cout << "Writing value of " << hex << int(data) << " to I8255 address of " << dec << int((addr & 0x3)) << endl;
			exit(0);
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

	    void set_mode(uint8_t data)
	    {
		control_reg = data;

		cout << "I8255 Group A mode: " << dec << int((control_reg >> 5) & 0x3) << endl;
		cout << "I8255 Port A mode: " << (testbit(control_reg, 4) ? "Input" : "Output") << endl;
		cout << "I8255 Port C upper mode: " << (testbit(control_reg, 3) ? "Input" : "Output") << endl;
		cout << "I8255 Group B mode: " << dec << int(testbit(control_reg, 2)) << endl;
		cout << "I8255 Port B mode: " << (testbit(control_reg, 1) ? "Input" : "Output") << endl;
		cout << "I8255 Port C lower mode: " << (testbit(control_reg, 0) ? "Input" : "Output") << endl;
		cout << endl;

		group_b_mode = testbit(control_reg, 2) ? 1 : 0;
		is_port_b_input = testbit(control_reg, 1);

		outputs[0] = 0;
		outputs[1] = 0;
		outputs[2] = 0;
	    }

	    void write_port_b(uint8_t data)
	    {
		switch (group_b_mode)
		{
		    case 0:
		    {
			if (!is_port_b_input)
			{
			    outputs[1] = data;

			    // TODO: Implement I8255 callback functions
			    cout << "Writing value of " << hex << int(data) << " to I8255 port B" << endl;
			}
		    }
		    break;
		    default:
		    {
			cout << "Unrecognized group B mode of " << dec << int(group_b_mode) << endl;
			exit(0);
		    }
		    break;
		}
	    }

	    uint8_t control_reg = 0;
	    int group_b_mode = 0;
	    bool is_port_b_input = false;

	    array<uint8_t, 3> outputs;
    };
};


#endif // LIBBLUEBERRN_I8255_PPI_H