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

#ifndef MB88XX_H
#define MB88XX_H

#include <iostream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <cassert>
using namespace std;

namespace mb88xx
{
    class mb88interface
    {
	public:
	    mb88interface()
	    {

	    }

	    ~mb88interface()
	    {

	    }

	    virtual uint8_t readROM(uint16_t addr) = 0;
	    virtual void writeMem(uint16_t addr, uint8_t data) = 0;
    };

    class mb88core
    {
	public:
	    mb88core(int prog_width, int data_width);
	    ~mb88core();

	    void set_interface(mb88interface *cb)
	    {
		inter = cb;
	    }

	    void init();
	    void shutdown();
	    void reset();
	    void debugoutput();
	    int runinstruction();
	    size_t disassemble(ostream &stream, size_t pc);

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
	    }

	    template<typename T>
	    T setbit(T reg, int bit)
	    {
		return (reg | (1 << bit));
	    }

	    template<typename T>
	    T resetbit(T reg, int bit)
	    {
		return (reg & ~(1 << bit));
	    }

	    template<typename T>
	    T changebit(T reg, int bit, bool val)
	    {
		return (val) ? setbit(reg, bit) : resetbit(reg, bit);
	    }

	    int program_width = 0;
	    int data_bus_width = 0;

	    uint8_t pa = 0;
	    uint8_t pc = 0;
	    uint8_t ind_a = 0;
	    uint8_t ind_x = 0;
	    uint8_t ind_y = 0;

	    uint16_t get_ea()
	    {
		return ((ind_x << 4) | ind_y);
	    }

	    uint16_t get_pc()
	    {
		return ((pa << 6) | pc);
	    }

	    void inc_pc()
	    {
		pc += 1;

		if (pc == 0x40)
		{
		    pc = 0;
		    pa += 1;
		}
	    }

	    bool test_st()
	    {
		return testbit(flags, 5);
	    }

	    void set_st(bool is_set)
	    {
		flags = changebit(flags, 5, is_set);
	    }

	    void set_zf(bool is_set)
	    {
		flags = changebit(flags, 4, is_set);
	    }

	    uint8_t readROM(uint16_t addr);
	    void writeMem(uint16_t addr, uint8_t data);

	    int executenextopcode(uint8_t opcode);
	    void unrecognizedopcode(uint8_t opcode);

	    uint8_t flags = 0;

	    mb88interface *inter = NULL;
    };

    class mb8843 : public mb88core
    {
	public:
	    mb8843() : mb88core(10, 6)
	    {

	    }

	    ~mb8843()
	    {

	    }
    };
};

#endif // MB88XX_H