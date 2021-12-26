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

#include "mb88xx.h"
using namespace mb88xx;

namespace mb88xx
{
    mb88core::mb88core(int prog_width, int data_width) : program_width(prog_width), data_bus_width(data_width)
    {

    }

    mb88core::~mb88core()
    {

    }

    void mb88core::init()
    {
	pa = 0;
	pc = 0;
	ind_x = 0;
	flags = 0x20;
	cout << "mb88core::Initialized" << endl;
    }

    void mb88core::shutdown()
    {
	cout << "mb88core::Shutting down..." << endl;
    }

    void mb88core::reset()
    {
	cout << "Resetting mb88core..." << endl;
	init();
    }

    uint8_t mb88core::readROM(uint16_t addr)
    {
	if (inter == NULL)
	{
	    return 0x00;
	}

	assert(addr < (1 << program_width));
	return inter->readROM(addr);
    }

    void mb88core::writeMem(uint16_t addr, uint8_t data)
    {
	if (inter == NULL)
	{
	    return;
	}

	assert(addr < (1 << data_bus_width));
	inter->writeMem(addr, data);
    }

    void mb88core::debugoutput()
    {
	cout << "PC: " << hex << int(pc) << endl;
	cout << "PA: " << hex << int(pa) << endl;
	cout << "A: " << hex << int(ind_a) << endl;
	cout << "X: " << hex << int(ind_x) << endl;
	cout << "Y: " << hex << int(ind_y) << endl;
	
	stringstream flags_str;
	flags_str << (testbit(flags, 5) ? "T" : "t");
	flags_str << (testbit(flags, 4) ? "Z" : "z");
	flags_str << (testbit(flags, 3) ? "C" : "c");
	flags_str << (testbit(flags, 2) ? "V" : "v");
	flags_str << (testbit(flags, 1) ? "S" : "s");
	flags_str << (testbit(flags, 0) ? "I" : "i");
	cout << "Flags: " << flags_str.str() << endl;
	stringstream instr_ss;
	disassemble(instr_ss, get_pc());
	cout << "Current instruction: " << instr_ss.str() << endl;
    }

    size_t mb88core::disassemble(ostream &stream, size_t pc)
    {
	uint32_t start_pc = pc;
	uint8_t opcode = readROM(pc++);
	uint8_t arg = readROM(pc);

	switch ((opcode & 0xF8))
	{
	    case 0x08:
	    {
		switch (opcode)
		{
		    case 0x0A: stream << "stic (M[X,Y] <= A; Y++)"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x18:
	    {
		switch (opcode)
		{
		    case 0x1B: stream << "xx (A<=>X)"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x58: stream << "lxi #$" << dec << int(opcode & 7) << " (X<=$" << dec << int(opcode & 7) << ")"; break;
	    case 0x68: stream << "jpl " << hex << int(opcode & 7) << hex << int(arg) << " (jump if st=1)"; pc += 1; break;
	    case 0x80:
	    case 0x88: stream << "lyi #$" << hex << int(opcode & 0xF) << " (Y<=$" << hex << int(opcode & 0xF) << ")"; break;
	    case 0x90:
	    case 0x98: stream << "li #$" << hex << int(opcode & 0xF) << " (A<=$" << hex << int(opcode & 0xF) << ")"; break;
	    case 0xC0:
	    case 0xC8:
	    case 0xD0:
	    case 0xD8:
	    case 0xE0:
	    case 0xE8:
	    case 0xF0:
	    case 0xF8:
	    {
		size_t addr = ((pc & ~0x3F) + (opcode - 0xC0));

		stream << "jmp $" << hex << int(addr) << " (jump if st=1)";
	    }
	    break;
	    default: stream << "und"; break;
	}

	return (pc - start_pc);
    }

    int mb88core::runinstruction()
    {
	uint8_t opcode = readROM(get_pc());
	inc_pc();
	return executenextopcode(opcode);
    }

    int mb88core::executenextopcode(uint8_t opcode)
    {
	// All but around 7 percent of the MB88xx instructions
	// run for 1 cycle
	int num_cycles = 1;

	switch ((opcode & 0xF8))
	{
	    case 0x08:
	    {
		switch (opcode)
		{
		    // stic ZCS:x.x
		    case 0x0A:
		    {
			writeMem(get_ea(), ind_a);
			set_st((ind_y & 0xF) != 0xF);
			ind_y = ((ind_y + 1) & 0xF);
			set_zf(ind_y == 0);
		    }
		    break;
		    default: unrecognizedopcode(opcode); break;
		}
	    }
	    break;
	    case 0x18:
	    {
		switch (opcode)
		{
		    // xx ZCS:x..
		    case 0x1B:
		    {
			uint8_t temp_x = ind_x;
			ind_x = ind_a;
			ind_a = temp_x;
			set_zf((ind_a == 0));
			set_st(true);
		    }
		    break;
		    default: unrecognizedopcode(opcode); break;
		}
	    }
	    break;
	    case 0x58:
	    {
		ind_x = (opcode & 7);
		set_zf((ind_x == 0));
		set_st(true);
	    }
	    break;
	    case 0x68:
	    {
		uint8_t addr = readROM(get_pc());
		inc_pc();
		num_cycles = 2;

		if (test_st())
		{
		    pc = (addr & 0x3F);
		    pa = (((opcode & 7) << 2) | (addr >> 6));
		}

		set_st(true);
	    }
	    break;
	    case 0x80:
	    case 0x88:
	    {
		ind_y = (opcode & 0xF);
		set_zf((ind_y == 0));
		set_st(true);
	    }
	    break;
	    case 0x90:
	    case 0x98:
	    {
		ind_a = (opcode & 0xF);
		set_zf((ind_a == 0));
		set_st(true);
	    }
	    break;
	    case 0xC0:
	    case 0xC8:
	    case 0xD0:
	    case 0xD8:
	    case 0xE0:
	    case 0xE8:
	    case 0xF0:
	    case 0xF8:
	    {
		if (test_st())
		{
		    pc = (opcode & 0x3F);
		}

		set_st(true);
	    }
	    break;
	    default: unrecognizedopcode(opcode); break;
	}

	return num_cycles;
    }

    void mb88core::unrecognizedopcode(uint8_t opcode)
    {
	cout << "Unrecognized MB88XX instruction of " << hex << int(opcode) << ", group of " << hex << int(opcode & 0xF8) << endl;
	exit(1);
    }
};