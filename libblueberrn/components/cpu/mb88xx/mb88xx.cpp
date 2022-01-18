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

#include "mb88xx.h"
using namespace mb88xx;

namespace mb88xx
{
    mb88xxInterface::mb88xxInterface()
    {

    }

    mb88xxInterface::~mb88xxInterface()
    {

    }

    mb88xxcore::mb88xxcore(int prog_width, int data_width) : program_width(prog_width), data_bus_width(data_width)
    {

    }

    mb88xxcore::~mb88xxcore()
    {

    }

    uint16_t mb88xxcore::get_pc()
    {
	return ((pa << 6) | pc);
    }

    void mb88xxcore::inc_pc()
    {
	pc += 1;

	if (pc >= 0x40)
	{
	    pc = 0;
	    pa += 1;
	}
    }

    uint16_t mb88xxcore::get_ea()
    {
	return ((ind_x << 4) | ind_y);
    }

    void mb88xxcore::init()
    {
	pa = 0;
	pc = 0;
	sp = 0;
	reg_a = 0;
	ind_x = 0;
	ind_y = 0;
	reg_pio = 0;
	reg_th = 0;
	reg_tl = 0;
	reg_sb = 0;
	reg_stack.fill(0);
	pending_irq = 0;
	clk_input = false;
	set_st(true);
	set_zf(false);
	set_cf(false);
	set_vf(false);
	set_sf(false);
	set_nf(false);
    }

    void mb88xxcore::shutdown()
    {
	return;
    }

    void mb88xxcore::clock_write(bool line)
    {
	if (clk_input != line)
	{
	    clk_input = line;

	    if (!clk_input && testbit(reg_pio, 6))
	    {
		increment_timer();
	    }
	}
    }

    void mb88xxcore::fire_irq(bool line)
    {
	if (testbit(reg_pio, irq_ext) && !test_nf() && line)
	{
	    pending_irq = setbit(pending_irq, irq_ext);
	}

	set_nf(line);
    }

    void mb88xxcore::increment_timer()
    {
	reg_tl += 1;

	if (reg_tl == 0x10)
	{
	    reg_tl = 0;
	    reg_th += 1;

	    if (reg_th == 0x10)
	    {
		reg_th = 0;
		set_vf(true);
		pending_irq = setbit(pending_irq, irq_timer);
	    }
	}
    }

    void mb88xxcore::setinterface(mb88xxInterface *cb)
    {
	inter = cb;
    }

    int mb88xxcore::runinstruction()
    {
	uint8_t opcode = readROM(get_pc());
	inc_pc();

	int op_cycles = executeopcode(opcode);
	int pio_cycles = update_pio(op_cycles);
	return (op_cycles + pio_cycles);
    }

    uint16_t mb88xxcore::get_irq_vec()
    {
	uint16_t value = get_pc();
	value |= (test_cf() << 13);
	value |= (test_zf() << 14);
	value |= (test_st() << 15);
	return value;
    }

    int mb88xxcore::update_pio(int cycles)
    {
	// TODO: Implement internal clock enable
	(void)cycles;

	uint8_t enabled_irqs = (pending_irq & reg_pio);

	if (enabled_irqs != 0)
	{
	    reg_stack.at(sp) = get_irq_vec();
	    sp = ((sp + 1) & 3);

	    // The datasheet doesn't mention interrupt vectors,
	    // but the Arabian MCU program expects the following
	    if (testbit(enabled_irqs, irq_ext))
	    {
		pc = 0x02;
	    }
	    else if (testbit(enabled_irqs, irq_timer))
	    {
		pc = 0x04;
	    }
	    else if (testbit(enabled_irqs, irq_serial))
	    {
		pc = 0x06;
	    }
  
	    pa = 0;
	    set_st(true);
	    pending_irq = 0;
	    return 3;
	}

	return 0;
    }

    uint8_t mb88xxcore::readROM(uint16_t addr)
    {
	if (inter == NULL)
	{
	    return 0x00;
	}

	assert(addr < (1 << program_width));
	return inter->readROM(addr);
    }

    uint8_t mb88xxcore::readMem(uint16_t addr)
    {
	if (inter == NULL)
	{
	    return 0x00;
	}

	int data_mask = ((1 << data_bus_width) - 1);
	addr &= data_mask;
	return inter->readMem(addr);
    }

    void mb88xxcore::writeMem(uint16_t addr, uint8_t data)
    {
	if (inter == NULL)
	{
	    return;
	}

	int data_mask = ((1 << data_bus_width) - 1);
	addr &= data_mask;
	inter->writeMem(addr, data);
    }

    uint8_t mb88xxcore::readR(int addr)
    {
	if (inter == NULL)
	{
	    return 0x00;
	}

	addr &= 3;
	return inter->readR(addr);
    }

    void mb88xxcore::writeR(int addr, uint8_t data)
    {
	if (inter == NULL)
	{
	    return;
	}

	addr &= 3;
	inter->writeR(addr, data);
    }

    uint8_t mb88xxcore::readK()
    {
	if (inter == NULL)
	{
	    return 0x00;
	}

	return inter->readK();
    }

    void mb88xxcore::writeO(uint8_t data)
    {
	if (inter == NULL)
	{
	    return;
	}

	inter->writeO(data);
    }

    void mb88xxcore::writeP(uint8_t data)
    {
	if (inter == NULL)
	{
	    return;
	}

	inter->writeP(data);
    }

    uint8_t mb88xxcore::pla(int inA, bool inB)
    {
	return ((inB << 4) | (inA & 0xF));
    }

    void mb88xxcore::updatePIOEnable(uint8_t new_pio)
    {
	reg_pio = new_pio;
    }

    void mb88xxcore::debugoutput()
    {
	cout << "PC: " << hex << int(pc) << endl;
	cout << "PA: " << hex << int(pa) << endl;
	cout << "A: " << hex << int(reg_a) << endl;
	cout << "X: " << hex << int(ind_x) << endl;
	cout << "Y: " << hex << int(ind_y) << endl;
	cout << "PIO: " << hex << int(reg_pio) << endl;
	cout << "TH: " << hex << int(reg_th) << endl;
	cout << "TL: " << hex << int(reg_tl) << endl;
	cout << "SB: " << hex << int(reg_sb) << endl;

	stringstream flags;
	flags << (test_st() ? "T" : "t");
	flags << (test_zf() ? "Z" : "z");
	flags << (test_cf() ? "C" : "c");
	flags << (test_vf() ? "V" : "v");
	flags << (test_sf() ? "S" : "s");
	flags << (test_nf() ? "I" : "i");

	cout << "Flags: " << flags.str() << endl;

	stringstream instr_dasm;
	disassembleinstr(instr_dasm, get_pc());
	cout << "Current instruction: " << instr_dasm.str() << endl;
    }

    size_t mb88xxcore::disassembleinstr(ostream &stream, size_t pc)
    {
	size_t startpc = pc;
	uint8_t opcode = readROM(pc++);
	uint8_t argument = readROM(pc);

	switch ((opcode & 0xF8))
	{
	    case 0x00:
	    {
		switch (opcode)
		{
		    case 0x00: stream << "nop"; break;
		    case 0x01: stream << "outO (PortO <= A)"; break;
		    case 0x02: stream << "outP"; break;
		    case 0x03: stream << "outR (PortR[Y] <= A)"; break;
		    case 0x04: stream << "tay (Y <= A)"; break;
		    case 0x05: stream << "tath (TH <= A)"; break;
		    case 0x06: stream << "tatl (SB <= A)"; break;
		    case 0x07: stream << "tas (SB <= A)"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x08:
	    {
		switch (opcode)
		{
		    case 0x08: stream << "icy (Y++)"; break;
		    case 0x09: stream << "icm ([M[X,Y]++)"; break;
		    case 0x0A: stream << "stic (M[X,Y] <= A; Y++)"; break;
		    case 0x0B: stream << "x (A <=> M[X,Y])"; break;
		    case 0x0C: stream << "rol"; break;
		    case 0x0D: stream << "load (A <=> M[X,Y])"; break;
		    case 0x0E: stream << "adc (A <= A + M[X,Y] + cf)"; break;
		    case 0x0F: stream << "and (A <= A & M[X,Y])"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x10:
	    {
		switch (opcode)
		{
		    case 0x10: stream << "daa (A <= A + 6 if (A > 9 | cf = 1))"; break;
		    case 0x11: stream << "das (A <= A + 10 if (A > 9 | cf = 1))"; break;
		    case 0x12: stream << "inK (A <= PortK)"; break;
		    case 0x13: stream << "inR (A <= PortR[Y])"; break;
		    case 0x14: stream << "tya (A <= Y)"; break;
		    case 0x15: stream << "ttha (A <= TH)"; break;
		    case 0x16: stream << "ttla (A <= TL)"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x18:
	    {
		switch (opcode)
		{
		    case 0x18: stream << "dcy (Y--)"; break;
		    case 0x19: stream << "dcm ([M[X,Y]--)"; break;
		    case 0x1A: stream << "stdc (M[X,Y] <= A; Y--)"; break;
		    case 0x1B: stream << "xx (A <=> X)"; break;
		    case 0x1C: stream << "ror"; break;
		    case 0x1D: stream << "store (M[X,Y] <= A)"; break;
		    case 0x1E: stream << "sbc (A <= M[X,Y] - A - cf)"; break;
		    case 0x1F: stream << "or (A <= A | M[X,Y])"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x20:
	    {
		switch (opcode)
		{
		    case 0x20: stream << "setR (PortR bit[Y] <= 1)"; break;
		    case 0x21: stream << "setc (cf <= 1)"; break;
		    case 0x22: stream << "rstR (PortR bit[Y] <= 0)"; break;
		    case 0x23: stream << "rstc (cf <= 0)"; break;
		    case 0x24: stream << "tstR (st <= PortR bit[Y])"; break;
		    case 0x25: stream << "tsti (st <= IRQ line)"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x28:
	    {
		switch (opcode)
		{
		    case 0x2C: stream << "rts"; break;
		    case 0x2D: stream << "neg (A = -A)"; break;
		    case 0x2E: stream << "c (A == M[X,Y])"; break;
		    case 0x2F: stream << "eor (A ^ M[X,Y])"; break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x30:
	    {
		switch (opcode)
		{
		    case 0x30:
		    case 0x31:
		    case 0x32:
		    case 0x33:
		    {
			int bit_num = (opcode & 3);
			stream << "sbit" << dec << bit_num << " (M[X,Y] bit " << dec << bit_num << " = 1)";
		    }
		    break;
		    case 0x34:
		    case 0x35:
		    case 0x36:
		    case 0x37:
		    {
			int bit_num = (opcode & 3);
			stream << "rbit" << dec << bit_num << " (M[X,Y] bit " << dec << bit_num << " = 0)";
		    }
		    break;
		}
	    }
	    break;
	    case 0x38:
	    {
		switch (opcode)
		{
		    case 0x38:
		    case 0x39:
		    case 0x3A:
		    case 0x3B:
		    {
			int bit_num = (opcode & 3);
			stream << "tbit" << dec << bit_num << " (M[X,Y] bit " << dec << bit_num << " == 1)";
		    }
		    break;
		    case 0x3C: stream << "rti"; break;
		    case 0x3D:
		    {
			stream << "jpa #$" << hex << int(argument) << " (jump always)";
			pc += 1;
		    }
		    break;
		    case 0x3E:
		    {
			stream << "en #$" << hex << int(argument) << " (enable bits)";
			pc += 1;
		    }
		    break;
		    case 0x3F:
		    {
			stream << "dis #$" << hex << int(argument) << " (enable bits)";
			pc += 1;
		    }
		    break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x48:
	    {
		switch (opcode)
		{
		    case 0x4C:
		    case 0x4D:
		    case 0x4E:
		    case 0x4F:
		    {
			int mem_val = (opcode & 3);
			stream << "tba" << dec << mem_val << " (A bit" << dec << mem_val << ")";
		    }
		    break;
		}
	    }
	    break;
	    case 0x50:
	    {
		switch (opcode)
		{
		    case 0x50:
		    case 0x51:
		    case 0x52:
		    case 0x53:
		    {
			int mem_val = (opcode & 3);
			stream << "xd" << dec << mem_val << " (A <=> M[0," << dec << mem_val << "])";
		    }
		    break;
		    case 0x54:
		    case 0x55:
		    case 0x56:
		    case 0x57:
		    {
			int mem_val = ((opcode & 3) + 4);
			stream << "xyd" << dec << mem_val << " (Y <=> M[0," << dec << mem_val << "])";
		    }
		    break;
		    default: stream << "und"; break;
		}
	    }
	    break;
	    case 0x58:
	    {
		int imm_val = (opcode & 7);
		stream << "lxi " << dec << imm_val << " (X <= " << dec << imm_val << ")";
	    }
	    break;
	    case 0x60:
	    {
		uint16_t call_addr = (((opcode & 7) << 8) | argument);
		stream << "call " << hex << int(call_addr) << " (call if st = 1)";
		pc += 1;
	    }
	    break;
	    case 0x68:
	    {
		uint16_t jmp_addr = (((opcode & 7) << 8) | argument);
		stream << "jpl " << hex << int(jmp_addr) << " (jump if st = 1)";
		pc += 1;
	    }
	    break;
	    case 0x70:
	    case 0x78:
	    {
		int imm_val = (opcode & 0xF);
		stream << "ai " << dec << imm_val << " (A <= A + " << dec << imm_val << ")";
	    }
	    break;
	    case 0x80:
	    case 0x88:
	    {
		int imm_val = (opcode & 0xF);
		stream << "lyi " << dec << imm_val << " (Y <= " << dec << imm_val << ")";
	    }
	    break;
	    case 0x90:
	    case 0x98:
	    {
		int imm_val = (opcode & 0xF);
		stream << "li " << dec << imm_val << " (A <= " << dec << imm_val << ")";
	    }
	    break;
	    case 0xA0:
	    case 0xA8:
	    {
		int imm_val = (opcode & 0xF);
		stream << "cyi " << dec << imm_val << " (Y == " << dec << imm_val << ")";
	    }
	    break;
	    case 0xB0:
	    case 0xB8:
	    {
		int imm_val = (opcode & 0xF);
		stream << "ci " << dec << imm_val << " (A == " << dec << imm_val << ")";
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
		uint16_t jmp_addr = ((pc & ~0x3F) + (opcode - 0xC0));
		stream << "jmp $" << hex << int(jmp_addr) << " (jump if st=1)";
	    }
	    break;
	    default: stream << "und"; break;
	}

	return (pc - startpc);
    }

    int mb88xxcore::executeopcode(uint8_t opcode)
    {
	// All but around 7 percent of the MB88XX instructions
	// take up 1 cycle
	int num_cycles = 1;

	switch ((opcode & 0xF8))
	{
	    case 0x00:
	    {
		switch (opcode)
		{
		    // nop ZCS:...
		    case 0x00:
		    {
			set_st(true);
		    }
		    break;
		    // outO ZCS:...
		    case 0x01:
		    {
			writeO(pla(reg_a, test_cf()));
			set_st(true);
		    }
		    break;
		    // outP ZCS:...
		    case 0x02:
		    {
			writeP(reg_a);
			set_st(true);
		    }
		    break;
		    // outR ZCS:...
		    case 0x03:
		    {
			int addr = (ind_y & 3);
			writeR(addr, reg_a);
			set_st(true);
		    }
		    break;
		    // tay ZCS:...
		    case 0x04:
		    {
			ind_y = reg_a;
			set_st(true);
		    }
		    break;
		    // tath ZCS:...
		    case 0x05:
		    {
			reg_th = reg_a;
			set_st(true);
		    }
		    break;
		    // tatl ZCS:...
		    case 0x06:
		    {
			reg_tl = reg_a;
			set_st(true);
		    }
		    break;
		    // tas ZCS:...
		    case 0x07:
		    {
			reg_sb = reg_a;
			set_st(true);
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x08:
	    {
		switch (opcode)
		{
		    // icy ZCS:x.x
		    case 0x08:
		    {
			set_st(ind_y < 0xF);
			ind_y = ((ind_y + 1) & 0xF);
			set_zf(ind_y == 0);
		    }
		    break;
		    // icm ZCS:x.x
		    case 0x09:
		    {
			uint8_t mem_val = readMem(get_ea());
			set_st(mem_val < 0xF);
			mem_val = ((mem_val + 1) & 0xF);
			set_zf(mem_val == 0);
			writeMem(get_ea(), mem_val);
		    }
		    break;
		    // stic ZCS:x.x
		    case 0x0A:
		    {
			writeMem(get_ea(), reg_a);
			set_st(ind_y < 0xF);
			ind_y = ((ind_y + 1) & 0xF);
			set_zf(ind_y == 0);
		    }
		    break;
		    // x ZCS:x..
		    case 0x0B:
		    {
			uint8_t mem_val = readMem(get_ea());
			writeMem(get_ea(), reg_a);
			reg_a = mem_val;
			set_zf(reg_a);
			set_st(true);
		    }
		    break;
		    // rol ZCS:xxx
		    case 0x0C:
		    {
			uint8_t value = ((reg_a << 1) | test_cf());
			set_cf(testbit(value, 4));
			set_st(!test_cf());
			reg_a = (value & 0xF);
			set_zf(reg_a == 0);
		    }
		    break;
		    // l ZCS:x..
		    case 0x0D:
		    {
			reg_a = readMem(get_ea());
			set_zf(reg_a == 0);
			set_st(true);
		    }
		    break;
		    // adc ZCS:xxx
		    case 0x0E:
		    {
			uint8_t mem_val = readMem(get_ea());
			uint8_t result = (mem_val + reg_a + test_cf());
			set_cf(testbit(result, 4));
			set_st(!test_cf());
			reg_a = (result & 0xF);
			set_zf(reg_a == 0);
		    }
		    break;
		    // and ZCS:x.x
		    case 0x0F:
		    {
			reg_a &= readMem(get_ea());
			set_zf(reg_a == 0);
			set_st(!test_zf());
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x10:
	    {
		switch (opcode)
		{
		    // daa ZCS:.xx
		    case 0x10:
		    {
			uint8_t value = reg_a;
			if (test_cf() || (reg_a > 9))
			{
			    value += 6;
			}

			set_cf(testbit(value, 4));
			set_st(!test_cf());
			reg_a = (value & 0xF);
		    }
		    break;
		    // das ZCS:.xx
		    case 0x11:
		    {
			uint8_t value = reg_a;
			if (test_cf() || (reg_a > 9))
			{
			    value += 10;
			}

			set_cf(testbit(value, 4));
			set_st(!test_cf());
			reg_a = (value & 0xF);
		    }
		    break;
		    // inK ZCS:x..
		    case 0x12:
		    {
			reg_a = (readK() & 0xF);
			set_zf(reg_a == 0);
			set_st(true);
		    }
		    break;
		    // inR ZCS:x..
		    case 0x13:
		    {
			int addr = (ind_y & 3);
			reg_a = (readR(addr) & 0xF);
			set_zf(reg_a == 0);
			set_st(true);
		    }
		    break;
		    // tya ZCS:x..
		    case 0x14:
		    {
			reg_a = ind_y;
			set_zf(reg_a);
			set_st(true);
		    }
		    break;
		    // ttha ZCS:x..
		    case 0x15:
		    {
			reg_a = reg_th;
			set_zf(reg_a == 0);
			set_st(true);
		    }
		    break;
		    // ttla ZCS:x..
		    case 0x16:
		    {
			reg_a = reg_tl;
			set_zf(reg_a == 0);
			set_st(true);
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x18:
	    {
		switch (opcode)
		{
		    // dcy ZCS:..x
		    case 0x18:
		    {
			set_st(ind_y != 0x0);
			ind_y = ((ind_y - 1) & 0xF);
		    }
		    break;
		    // dcm ZCS:x.x
		    case 0x19:
		    {
			uint8_t mem_val = readMem(get_ea());
			set_st(mem_val != 0x0);
			mem_val = ((mem_val - 1) & 0xF);
			set_zf(mem_val == 0);
			writeMem(get_ea(), mem_val);
		    }
		    break;
		    // stdc ZCS:x.x
		    case 0x1A:
		    {
			writeMem(get_ea(), reg_a);
			set_st(ind_y != 0x0);
			ind_y = ((ind_y - 1) & 0xF);
			set_zf(ind_y == 0);
		    }
		    break;
		    // xx ZCS:x..
		    case 0x1B:
		    {
			uint8_t temp_x = ind_x;
			ind_x = reg_a;
			reg_a = temp_x;
			set_zf(reg_a == 0);
			set_st(true);
		    }
		    break;
		    // ror ZCS:xxx
		    case 0x1C:
		    {
			bool prev_carry = test_cf();
			set_cf(testbit(reg_a, 0));
			set_st(!test_cf());
			uint8_t value = ((reg_a >> 1) | (prev_carry << 3));
			reg_a = (value & 0xF);
			set_zf(reg_a == 0);
		    }
		    break;
		    // st ZCS:x..
		    case 0x1D:
		    {
			writeMem(get_ea(), reg_a);
			set_st(true);
		    }
		    break;
		    // sbc ZCS:xxx
		    case 0x1E:
		    {
			uint8_t mem_val = readMem(get_ea());
			uint8_t result = (mem_val - reg_a - test_cf());
			set_cf(testbit(result, 4));
			set_st(!test_cf());
			reg_a = (result & 0xF);
			set_zf(reg_a == 0);
		    }
		    break;
		    // or ZCS:x.x
		    case 0x1F:
		    {
			reg_a |= readMem(get_ea());
			set_zf(reg_a == 0);
			set_st(!test_zf());
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x20:
	    {
		switch (opcode)
		{
		    // setR ZCS:...
		    case 0x20:
		    {
			int addr = (ind_y / 4);
			int bit = (ind_y % 4);
			uint8_t result = readR(addr);
			writeR(addr, setbit(result, bit));
			set_st(true);
		    }
		    break;
		    // setc ZCX:.xx
		    case 0x21:
		    {
			set_cf(true);
			set_st(true);
		    }
		    break;
		    // rstR ZCS:...
		    case 0x22:
		    {
			int addr = (ind_y / 4);
			int bit = (ind_y % 4);
			uint8_t result = readR(addr);
			writeR(addr, resetbit(result, bit));
			set_st(true);
		    }
		    break;
		    // rstc ZCS:.xx
		    case 0x23:
		    {
			set_cf(false);
			set_st(true);
		    }
		    break;
		    // tstR ZCS:..x
		    case 0x24:
		    {
			int addr = (ind_y / 4);
			int bit = (ind_y % 4);
			uint8_t result = readR(addr);
			set_st(!testbit(result, bit));
		    }
		    break;
		    // tsti ZCS:..x
		    case 0x25:
		    {
			set_st(!test_nf());
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x28:
	    {
		switch (opcode)
		{
		    // rts ZCS:...
		    case 0x2C:
		    {
			sp = ((sp - 1) & 3);
			uint16_t value = reg_stack.at(sp);
			pc = (value & 0x3F);
			pa = ((value >> 6) & 0x1F);
			set_st(true);
		    }
		    break;
		    // neg ZCS:..x
		    case 0x2D:
		    {
			reg_a = ((0 - reg_a) & 0xF);
			set_st(reg_a != 0);
		    }
		    break;
		    // c ZCS:xxx
		    case 0x2E:
		    {
			uint8_t mem_val = readMem(get_ea());
			uint8_t res = (mem_val - reg_a);
			set_cf(testbit(res, 4));
			set_zf((res & 0xF) == 0);
			set_st(!test_zf());
		    }
		    break;
		    // eor ZCS:x.x
		    case 0x2F:
		    {
			uint8_t mem_val = readMem(get_ea());
			reg_a ^= mem_val;
			set_zf(reg_a == 0);
			set_st(!test_zf());
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x30:
	    {
		switch (opcode)
		{
		    // sbit ZCS:...
		    case 0x30:
		    case 0x31:
		    case 0x32:
		    case 0x33:
		    {
			int bit_num = (opcode & 3);
			uint8_t mem_val = readMem(get_ea());
			writeMem(get_ea(), setbit(mem_val, bit_num));
			set_st(true);
		    }
		    break;
		    // rbit ZCS:...
		    case 0x34:
		    case 0x35:
		    case 0x36:
		    case 0x37:
		    {
			int bit_num = (opcode & 3);
			uint8_t mem_val = readMem(get_ea());
			writeMem(get_ea(), resetbit(mem_val, bit_num));
			set_st(true);
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x38:
	    {
		switch (opcode)
		{
		    // tbit ZCS:...
		    case 0x38:
		    case 0x39:
		    case 0x3A:
		    case 0x3B:
		    {
			uint8_t mem_val = readMem(get_ea());
			int bit_num = (opcode & 3);
			set_st(!testbit(mem_val, bit_num));
		    }
		    break;
		    // rti ZCS:...
		    case 0x3C:
		    {
			sp = ((sp - 1) & 3);
			uint16_t value = reg_stack.at(sp);
			pc = (value & 0x3F);
			pa = ((value >> 6) & 0x1F);
			set_cf(testbit(value, 13));
			set_zf(testbit(value, 14));
			set_st(testbit(value, 15));
		    }
		    break;
		    // jpa imm ZCS:..x
		    case 0x3D:
		    {
			uint8_t addr = readROM(get_pc());
			pa = (addr & 0x1F);
			pc = (reg_a * 4);
			num_cycles = 2;
			set_st(true);
		    }
		    break;
		    // en imm ZCS:...
		    case 0x3E:
		    {
			uint8_t enable_bits = readROM(get_pc());
			inc_pc();
			updatePIOEnable(reg_pio | enable_bits);
			num_cycles = 2;
			set_st(true);
		    }
		    break;
		    // dis imm ZCS:...
		    case 0x3F:
		    {
			uint8_t enable_bits = readROM(get_pc());
			inc_pc();
			updatePIOEnable(reg_pio & ~enable_bits);
			num_cycles = 2;
			set_st(true);
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x48:
	    {
		switch (opcode)
		{
		    case 0x4C:
		    case 0x4D:
		    case 0x4E:
		    case 0x4F:
		    {
			int op_bit = (opcode & 3);
			set_st(!testbit(reg_a, op_bit));
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    case 0x50:
	    {
		switch (opcode)
		{
		    // xd ZCS:x..
		    case 0x50:
		    case 0x51:
		    case 0x52:
		    case 0x53:
		    {
			int addr = (opcode & 3);
			uint8_t mem_val = readMem(addr);
			writeMem(addr, reg_a);
			reg_a = mem_val;
			set_zf(reg_a == 0);
			set_st(true);
		    }
		    break;
		    // xyd ZCS:x..
		    case 0x54:
		    case 0x55:
		    case 0x56:
		    case 0x57:
		    {
			int addr = ((opcode & 3) + 4);
			uint8_t mem_val = readMem(addr);
			writeMem(addr, ind_y);
			ind_y = mem_val;
			set_zf(ind_y == 0);
			set_st(true);
		    }
		    break;
		    default: unrecognizedinstr(opcode); break;
		}
	    }
	    break;
	    // lxi ZCS:x..
	    case 0x58:
	    {
		ind_x = (opcode & 7);
		set_zf(ind_x == 0);
		set_st(true);
	    }
	    break;
	    // call imm ZCS:..x
	    case 0x60:
	    {
		uint8_t call_arg = readROM(get_pc());
		inc_pc();
		num_cycles = 2;

		if (test_st())
		{
		    reg_stack.at(sp) = get_pc();
		    sp = ((sp + 1) & 3);
		    pc = (call_arg & 0x3F);
		    pa = (((opcode & 7) << 2) | (call_arg >> 6));
		}

		set_st(true);
	    }
	    break;
	    // jpl imm ZCS:..x
	    case 0x68:
	    {
		uint8_t jmp_arg = readROM(get_pc());
		inc_pc();
		num_cycles = 2;

		if (test_st())
		{
		    pc = (jmp_arg & 0x3F);
		    pa = (((opcode & 7) << 2) | (jmp_arg >> 6));
		}

		set_st(true);
	    }
	    break;
	    // ai ZCS:xxx
	    case 0x70:
	    case 0x78:
	    {
		uint8_t res = ((opcode & 0xF) + reg_a);
		set_cf(testbit(res, 4));
		set_st(!test_cf());
		reg_a = (res & 0xF);
		set_zf(reg_a == 0);
	    }
	    break;
	    // lyi ZCS:x..
	    case 0x80:
	    case 0x88:
	    {
		ind_y = (opcode & 0xF);
		set_zf(ind_y == 0);
		set_st(true);
	    }
	    break;
	    // li ZCS:x..
	    case 0x90:
	    case 0x98:
	    {
		reg_a = (opcode & 0xF);
		set_zf(reg_a == 0);
		set_st(true);
	    }
	    break;
	    // cyi ZCS:xxx
	    case 0xA0:
	    case 0xA8:
	    {
		uint8_t res = ((opcode & 0xF) - ind_y);
		set_cf(testbit(res, 4));
		set_zf((res & 0xF) == 0);
		set_st(!test_zf());
	    }
	    break;
	    // ci ZCS:xxx
	    case 0xB0:
	    case 0xB8:
	    {
		uint8_t res = ((opcode & 0xF) - reg_a);
		set_cf(testbit(res, 4));
		set_zf((res & 0xF) == 0);
		set_st(!test_zf());
	    }
	    break;
	    // jmp ZCS:..x
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
	    default: unrecognizedinstr(opcode); break;
	}

	return num_cycles;
    }

    void mb88xxcore::unrecognizedinstr(uint8_t opcode)
    {
	cout << "Unrecognized MB88XX opcode of " << hex << int(opcode) << ", group of " << hex << int((opcode & 0xF8)) << endl;
	exit(1);
    }
};