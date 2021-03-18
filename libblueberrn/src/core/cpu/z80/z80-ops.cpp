#include "z80.h"
using namespace zilogz80;

namespace zilogz80
{
    int z80::executeopcode(uint8_t opcode)
    {
	int cycles = 0;        

	switch (opcode)
        {
            case 0x00: cycles = 4; break;
            case 0x01: bc.reg = getimmword(); cycles = 10; break;
	    case 0x03: bc.reg += 1; cycles = 6; break;
	    case 0x06: bc.hi = getimmbyte(); cycles = 7; break;
	    case 0x0E: bc.lo = getimmbyte(); cycles = 7; break;
	    case 0x10: cycles = djnz(getimmsignedbyte()); cycles = 10; break;
	    case 0x21: hl.reg = getimmword(); cycles = 10; break;
	    case 0x23: hl.reg += 1; cycles = 6; break;
	    case 0x24: hl.hi = increg(hl.hi); cycles = 4; break;
	    case 0x2C: hl.lo = increg(hl.lo); cycles = 4; break;
	    case 0x32: writeByte(getimmword(), af.hi); cycles = 13; break;
	    case 0x3A: af.hi = readByte(readWord(getimmword())); cycles = 13; break;
	    case 0x3C: af.hi = increg(af.hi); cycles = 4; break;
	    case 0x3D: af.hi = decreg(af.hi); cycles = 4; break;
	    case 0x3E: af.hi = getimmbyte(); cycles = 7; break;
            case 0x40: bc.hi = bc.hi; cycles = 4; break;
            case 0x41: bc.hi = bc.lo; cycles = 4; break;
            case 0x42: bc.hi = de.hi; cycles = 4; break;
            case 0x43: bc.hi = de.lo; cycles = 4; break;
            case 0x44: bc.hi = hl.hi; cycles = 4; break;
            case 0x45: bc.hi = hl.lo; cycles = 4; break;
            case 0x46: bc.hi = readByte(hl.reg); cycles = 7; break;
            case 0x47: bc.hi = af.hi; cycles = 4; break;
            case 0x48: bc.lo = bc.hi; cycles = 4; break;
            case 0x49: bc.lo = bc.lo; cycles = 4; break;
            case 0x4A: bc.lo = de.hi; cycles = 4; break;
            case 0x4B: bc.lo = de.lo; cycles = 4; break;
            case 0x4C: bc.lo = hl.hi; cycles = 4; break;
            case 0x4D: bc.lo = hl.lo; cycles = 4; break;
            case 0x4E: bc.lo = readByte(hl.reg); cycles = 7; break;
            case 0x4F: bc.lo = af.hi; cycles = 4; break;
            case 0x50: de.hi = bc.hi; cycles = 4; break;
            case 0x51: de.hi = bc.lo; cycles = 4; break;
            case 0x52: de.hi = de.hi; cycles = 4; break;
            case 0x53: de.hi = de.lo; cycles = 4; break;
            case 0x54: de.hi = hl.hi; cycles = 4; break;
            case 0x55: de.hi = hl.lo; cycles = 4; break;
            case 0x56: de.hi = readByte(hl.reg); cycles = 7; break;
            case 0x57: de.hi = af.hi; cycles = 4; break;
            case 0x58: de.lo = bc.hi; cycles = 4; break;
            case 0x59: de.lo = bc.lo; cycles = 4; break;
            case 0x5A: de.lo = de.hi; cycles = 4; break;
            case 0x5B: de.lo = de.lo; cycles = 4; break;
            case 0x5C: de.lo = hl.hi; cycles = 4; break;
            case 0x5D: de.lo = hl.lo; cycles = 4; break;
            case 0x5E: de.lo = readByte(hl.reg); cycles = 7; break;
            case 0x5F: de.lo = af.hi; cycles = 4; break;
            case 0x60: hl.hi = bc.hi; cycles = 4; break;
            case 0x61: hl.hi = bc.lo; cycles = 4; break;
            case 0x62: hl.hi = de.hi; cycles = 4; break;
            case 0x63: hl.hi = de.lo; cycles = 4; break;
            case 0x64: hl.hi = hl.hi; cycles = 4; break;
            case 0x65: hl.hi = hl.lo; cycles = 4; break;
            case 0x66: hl.hi = readByte(hl.reg); cycles = 7; break;
            case 0x67: hl.hi = af.hi; cycles = 4; break;
            case 0x68: hl.lo = bc.hi; cycles = 4; break;
            case 0x69: hl.lo = bc.lo; cycles = 4; break;
            case 0x6A: hl.lo = de.hi; cycles = 4; break;
            case 0x6B: hl.lo = de.lo; cycles = 4; break;
            case 0x6C: hl.lo = hl.hi; cycles = 4; break;
            case 0x6D: hl.lo = hl.lo; cycles = 4; break;
            case 0x6E: hl.lo = readByte(hl.reg); cycles = 7; break;
            case 0x6F: hl.lo = af.hi; cycles = 4; break;
            case 0x70: writeByte(hl.reg, bc.hi); cycles = 7; break;
            case 0x71: writeByte(hl.reg, bc.lo); cycles = 7; break;
            case 0x72: writeByte(hl.reg, de.hi); cycles = 7; break;
            case 0x73: writeByte(hl.reg, de.lo); cycles = 7; break;
            case 0x74: writeByte(hl.reg, hl.hi); cycles = 7; break;
            case 0x75: writeByte(hl.reg, hl.lo); cycles = 7; break;
            case 0x76: unrecognizedopcode(opcode); break;
            case 0x77: writeByte(hl.reg, af.hi); cycles = 7; break;
            case 0x78: af.hi = bc.hi; cycles = 4; break;
            case 0x79: af.hi = bc.lo; cycles = 4; break;
            case 0x7A: af.hi = de.hi; cycles = 4; break;
            case 0x7B: af.hi = de.lo; cycles = 4; break;
            case 0x7C: af.hi = hl.hi; cycles = 4; break;
            case 0x7D: af.hi = hl.lo; cycles = 4; break;
            case 0x7E: af.hi = readByte(hl.reg); cycles = 7; break;
            case 0x7F: af.hi = af.hi; cycles = 4; break;
	    case 0xA8: af.hi = xorreg(af.hi, bc.hi); cycles = 4; break;
	    case 0xA9: af.hi = xorreg(af.hi, bc.lo); cycles = 4; break;
	    case 0xAA: af.hi = xorreg(af.hi, de.hi); cycles = 4; break;
	    case 0xAB: af.hi = xorreg(af.hi, de.lo); cycles = 4; break;
	    case 0xAC: af.hi = xorreg(af.hi, hl.hi); cycles = 4; break;
	    case 0xAD: af.hi = xorreg(af.hi, hl.lo); cycles = 4; break;
	    case 0xAE: af.hi = xorreg(af.hi, readByte(hl.reg)); cycles = 7; break;
	    case 0xAF: af.hi = xorreg(af.hi, af.hi); cycles = 4; break;
	    case 0xC2: cycles = jumpcond(getimmword(), !iszero()); break;
	    case 0xC3: cycles = jump(getimmword()); break;
	    case 0xC6: af.hi = addreg(af.hi, getimmbyte()); cycles = 7; break;
	    case 0xEB: cycles = exchangedehl(); break;
            default: unrecognizedopcode(opcode); break;
        }

	return cycles;
    }
}
