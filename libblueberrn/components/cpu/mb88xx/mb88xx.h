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

#ifndef MB88XX_H
#define MB88XX_H

#include <iostream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <array>
#include <cassert>
using namespace std;

namespace mb88xx
{
    class mb88xxInterface
    {
	public:
	    mb88xxInterface();
	    ~mb88xxInterface();

	    virtual uint8_t readROM(uint16_t addr) = 0;
	    virtual uint8_t readMem(uint16_t addr) = 0;
	    virtual void writeMem(uint16_t addr, uint8_t data) = 0;
	    virtual uint8_t readR(int addr) = 0;
	    virtual void writeR(int addr, uint8_t data) = 0;
	    virtual uint8_t readK() = 0;
	    virtual void writeO(uint8_t data) = 0;
	    virtual void writeP(uint8_t data) = 0;
    };

    class mb88xxcore
    {
	public:
	    mb88xxcore(int prog_width, int data_width);
	    ~mb88xxcore();

	    void setinterface(mb88xxInterface *cb);
	    void debugoutput();

	    void init();
	    void shutdown();

	    void clock_write(bool line);
	    void fire_irq(bool line);

	    int runinstruction();
	    size_t disassembleinstr(ostream &stream, size_t pc);

	    bool dump = false;

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
		if (val)
		{
		    return setbit(reg, bit);
		}
		else
		{
		    return resetbit(reg, bit);
		}
	    }

	    void increment_timer();

	    int program_width = 0;
	    int data_bus_width = 0;

	    uint8_t pa = 0;
	    uint8_t pc = 0;
	    uint8_t sp = 0;
	    uint8_t reg_a = 0;
	    uint8_t ind_x = 0;
	    uint8_t ind_y = 0;
	    uint8_t reg_pio = 0;
	    uint8_t reg_th = 0;
	    uint8_t reg_tl = 0;
	    uint8_t reg_sb = 0;
	    array<uint16_t, 4> reg_stack;

	    uint8_t pending_irq = 0;

	    bool clk_input = false;

	    uint8_t flags = 0;

	    bool test_st()
	    {
		return testbit(flags, 5);
	    }

	    void set_st(bool val)
	    {
		flags = changebit(flags, 5, val);
	    }

	    bool test_zf()
	    {
		return testbit(flags, 4);
	    }

	    void set_zf(bool val)
	    {
		flags = changebit(flags, 4, val);
	    }

	    bool test_cf()
	    {
		return testbit(flags, 3);
	    }

	    void set_cf(bool val)
	    {
		flags = changebit(flags, 3, val);
	    }

	    bool test_vf()
	    {
		return testbit(flags, 2);
	    }

	    void set_vf(bool val)
	    {
		flags = changebit(flags, 2, val);
	    }

	    bool test_sf()
	    {
		return testbit(flags, 1);
	    }

	    void set_sf(bool val)
	    {
		flags = changebit(flags, 1, val);
	    }

	    bool test_nf()
	    {
		return testbit(flags, 0);
	    }

	    void set_nf(bool val)
	    {
		flags = changebit(flags, 0, val);
	    }

	    uint16_t get_pc();
	    void inc_pc();
	    uint16_t get_ea();

	    int executeopcode(uint8_t opcode);
	    void unrecognizedinstr(uint8_t opcode);

	    uint8_t readROM(uint16_t addr);
	    uint8_t readMem(uint16_t addr);
	    void writeMem(uint16_t addr, uint8_t data);
	    uint8_t readR(int addr);
	    void writeR(int addr, uint8_t data);
	    uint8_t readK();
	    void writeO(uint8_t data);
	    void writeP(uint8_t data);

	    uint8_t pla(int inA, bool inB);

	    void updatePIOEnable(uint8_t new_pio);
	    int update_pio(int cycles);
	    uint16_t get_irq_vec();

	    const int irq_serial = 0;
	    const int irq_timer = 1;
	    const int irq_ext = 2;

	    mb88xxInterface *inter = NULL;
    };

    class mb8843core : public mb88xxcore
    {
	public:
	    mb8843core() : mb88xxcore(10, 6)
	    {

	    }

	    ~mb8843core()
	    {

	    }

	    void init()
	    {
		mb88xxcore::init();
		cout << "MB8843::Intialized" << endl;
	    }

	    void shutdown()
	    {
		mb88xxcore::shutdown();
		cout << "MB8843::Shutting down..." << endl;
	    }

	    void reset()
	    {
		cout << "Resetting MB8843..." << endl;
		init();
	    }
    };

    class mb8844core : public mb88xxcore
    {
	public:
	    mb8844core() : mb88xxcore(10, 6)
	    {

	    }

	    ~mb8844core()
	    {

	    }

	    void init()
	    {
		mb88xxcore::init();
		cout << "MB8844::Intialized" << endl;
	    }

	    void shutdown()
	    {
		mb88xxcore::shutdown();
		cout << "MB8844::Shutting down..." << endl;
	    }

	    void reset()
	    {
		cout << "Resetting MB8844..." << endl;
		init();
	    }
    };
};

#endif // MB88XX_H