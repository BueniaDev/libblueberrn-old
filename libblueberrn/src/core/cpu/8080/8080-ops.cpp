#include "8080.h"
using namespace intel8080;

namespace intel8080
{
    int i8080::executeopcode(uint8_t opcode)
    {
        int cycles = 0;

	switch (opcode)
        {
            case 0x00: cycles = 4; break;
            case 0x01: bc.reg = getimmword(); cycles = 10; break;
	    case 0x02: writeByte(bc.reg, af.hi); cycles = 7; break;
	    case 0x03: bc.reg += 1; cycles = 5; break;
	    case 0x04: bc.hi = inr(bc.hi); cycles = 5; break;
	    case 0x05: bc.hi = dcr(bc.hi); cycles = 5; break;
            case 0x06: bc.hi = getimmbyte(); cycles = 7; break;
	    case 0x07: rlcaccum(); cycles = 4; break;
            case 0x08: cycles = 4; break;
	    case 0x09: hl.reg = addreg(hl.reg, bc.reg); cycles = 10; break;
	    case 0x0A: af.hi = readByte(bc.reg); cycles = 7; break;
	    case 0x0B: bc.reg -= 1; cycles = 5; break;
	    case 0x0C: bc.lo = inr(bc.lo); cycles = 5; break;
	    case 0x0D: bc.lo = dcr(bc.lo); cycles = 5; break;
            case 0x0E: bc.lo = getimmbyte(); cycles = 7; break;
	    case 0x0F: rrcaccum(); cycles = 4; break;
            case 0x10: cycles = 4; break;
            case 0x11: de.reg = getimmword(); cycles = 10; break;
	    case 0x12: writeByte(de.reg, af.hi); cycles = 7; break; 
	    case 0x13: de.reg += 1; cycles = 5; break;
	    case 0x14: de.hi = inr(de.hi); cycles = 5; break;
	    case 0x15: de.hi = dcr(de.hi); cycles = 5; break;
            case 0x16: de.hi = getimmbyte(); cycles = 7; break;
	    case 0x17: ralaccum(); cycles = 4; break;
            case 0x18: cycles = 4; break;
	    case 0x19: hl.reg = addreg(hl.reg, de.reg); cycles = 10; break;
	    case 0x1A: af.hi = readByte(de.reg); cycles = 7; break;
	    case 0x1B: de.reg -= 1; cycles = 5; break;
	    case 0x1C: de.lo = inr(de.lo); cycles = 5; break;
	    case 0x1D: de.lo = dcr(de.lo); cycles = 5; break;
            case 0x1E: de.lo = getimmbyte(); cycles = 7; break;
	    case 0x1F: raraccum(); cycles = 4; break;
            case 0x20: cycles = 4; break;
            case 0x21: hl.reg = getimmword(); cycles = 10; break;
	    case 0x22: writeWord(getimmword(), hl.reg); cycles = 16; break;
	    case 0x23: hl.reg += 1; cycles = 5; break;
	    case 0x24: hl.hi = inr(hl.hi); cycles = 5; break;
	    case 0x25: hl.hi = dcr(hl.hi); cycles = 5; break;
            case 0x26: hl.hi = getimmbyte(); cycles = 7; break;
	    case 0x27: daa(); cycles = 4; break;
            case 0x28: cycles = 4; break;
	    case 0x29: hl.reg = addreg(hl.reg, hl.reg); cycles = 10; break;
	    case 0x2A: hl.reg = readWord(getimmword()); cycles = 16; break;
	    case 0x2B: hl.reg -= 1; cycles = 5; break;
	    case 0x2C: hl.lo = inr(hl.lo); cycles = 5; break;
	    case 0x2D: hl.lo = dcr(hl.lo); cycles = 5; break;
            case 0x2E: hl.lo = getimmbyte(); cycles = 7; break;
	    case 0x2F: af.hi ^= 0xFF; cycles = 4; break;
            case 0x30: cycles = 4; break;
            case 0x31: sp = getimmword(); cycles = 10; break;
	    case 0x32: writeByte(getimmword(), af.hi); cycles = 13; break;
	    case 0x33: sp += 1; cycles = 5; break;
	    case 0x34: writeByte(hl.reg, inr(readByte(hl.reg))); cycles = 10; break;
	    case 0x35: writeByte(hl.reg, dcr(readByte(hl.reg))); cycles = 10; break;
            case 0x36: writeByte(hl.reg, getimmbyte()); cycles = 10; break;
	    case 0x37: setcarry(true); cycles = 4; break;
            case 0x38: cycles = 4; break;
	    case 0x39: hl.reg = addreg(hl.reg, sp); cycles = 10; break;
	    case 0x3A: af.hi = readByte(getimmword()); cycles = 13; break;
	    case 0x3B: sp -= 1; cycles = 5; break;
	    case 0x3C: af.hi = inr(af.hi); cycles = 5; break;
	    case 0x3D: af.hi = dcr(af.hi); cycles = 5; break;
            case 0x3E: af.hi = getimmbyte(); cycles = 7; break;
	    case 0x3F: setcarry(!iscarry()); cycles = 4; break;
            case 0x40: bc.hi = bc.hi; cycles = 5; break;
            case 0x41: bc.hi = bc.lo; cycles = 5; break;
            case 0x42: bc.hi = de.hi; cycles = 5; break;
            case 0x43: bc.hi = de.lo; cycles = 5; break;
            case 0x44: bc.hi = hl.hi; cycles = 5; break;
            case 0x45: bc.hi = hl.lo; cycles = 5; break;
            case 0x46: bc.hi = readByte(hl.reg); cycles = 7; break;
            case 0x47: bc.hi = af.hi; cycles = 5; break;
            case 0x48: bc.lo = bc.hi; cycles = 5; break;
            case 0x49: bc.lo = bc.lo; cycles = 5; break;
            case 0x4A: bc.lo = de.hi; cycles = 5; break;
            case 0x4B: bc.lo = de.lo; cycles = 5; break;
            case 0x4C: bc.lo = hl.hi; cycles = 5; break;
            case 0x4D: bc.lo = hl.lo; cycles = 5; break;
            case 0x4E: bc.lo = readByte(hl.reg); cycles = 7; break;
            case 0x4F: bc.lo = af.hi; cycles = 5; break;
            case 0x50: de.hi = bc.hi; cycles = 5; break;
            case 0x51: de.hi = bc.lo; cycles = 5; break;
            case 0x52: de.hi = de.hi; cycles = 5; break;
            case 0x53: de.hi = de.lo; cycles = 5; break;
            case 0x54: de.hi = hl.hi; cycles = 5; break;
            case 0x55: de.hi = hl.lo; cycles = 5; break;
            case 0x56: de.hi = readByte(hl.reg); cycles = 7; break;
            case 0x57: de.hi = af.hi; cycles = 5; break;
            case 0x58: de.lo = bc.hi; cycles = 5; break;
            case 0x59: de.lo = bc.lo; cycles = 5; break;
            case 0x5A: de.lo = de.hi; cycles = 5; break;
            case 0x5B: de.lo = de.lo; cycles = 5; break;
            case 0x5C: de.lo = hl.hi; cycles = 5; break;
            case 0x5D: de.lo = hl.lo; cycles = 5; break;
            case 0x5E: de.lo = readByte(hl.reg); cycles = 7; break;
            case 0x5F: de.lo = af.hi; cycles = 5; break;
            case 0x60: hl.hi = bc.hi; cycles = 5; break;
            case 0x61: hl.hi = bc.lo; cycles = 5; break;
            case 0x62: hl.hi = de.hi; cycles = 5; break;
            case 0x63: hl.hi = de.lo; cycles = 5; break;
            case 0x64: hl.hi = hl.hi; cycles = 5; break;
            case 0x65: hl.hi = hl.lo; cycles = 5; break;
            case 0x66: hl.hi = readByte(hl.reg); cycles = 7; break;
            case 0x67: hl.hi = af.hi; cycles = 5; break;
            case 0x68: hl.lo = bc.hi; cycles = 5; break;
            case 0x69: hl.lo = bc.lo; cycles = 5; break;
            case 0x6A: hl.lo = de.hi; cycles = 5; break;
            case 0x6B: hl.lo = de.lo; cycles = 5; break;
            case 0x6C: hl.lo = hl.hi; cycles = 5; break;
            case 0x6D: hl.lo = hl.lo; cycles = 5; break;
            case 0x6E: hl.lo = readByte(hl.reg); cycles = 7; break;
            case 0x6F: hl.lo = af.hi; cycles = 5; break;
            case 0x70: writeByte(hl.reg, bc.hi); cycles = 7; break;
            case 0x71: writeByte(hl.reg, bc.lo); cycles = 7; break;
            case 0x72: writeByte(hl.reg, de.hi); cycles = 7; break;
            case 0x73: writeByte(hl.reg, de.lo); cycles = 7; break;
            case 0x74: writeByte(hl.reg, hl.hi); cycles = 7;  break;
            case 0x75: writeByte(hl.reg, hl.lo); cycles = 7; break;
            case 0x76: pc--; cycles = 7; break;
            case 0x77: writeByte(hl.reg, af.hi); cycles = 7; break;
            case 0x78: af.hi = bc.hi; cycles = 5; break;
            case 0x79: af.hi = bc.lo; cycles = 5; break;
            case 0x7A: af.hi = de.hi; cycles = 5; break;
            case 0x7B: af.hi = de.lo; cycles = 5; break;
            case 0x7C: af.hi = hl.hi; cycles = 5; break;
            case 0x7D: af.hi = hl.lo; cycles = 5; break;
            case 0x7E: af.hi = readByte(hl.reg); cycles = 7; break;
            case 0x7F: af.hi = af.hi; cycles = 5; break;
            case 0x80: af.hi = add(af.hi, bc.hi); cycles = 4; break;
            case 0x81: af.hi = add(af.hi, bc.lo); cycles = 4; break;
            case 0x82: af.hi = add(af.hi, de.hi); cycles = 4; break;
            case 0x83: af.hi = add(af.hi, de.lo); cycles = 4; break;
            case 0x84: af.hi = add(af.hi, hl.hi); cycles = 4; break;
            case 0x85: af.hi = add(af.hi, hl.lo); cycles = 4; break;
            case 0x86: af.hi = add(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0x87: af.hi = add(af.hi, af.hi); cycles = 4; break;
            case 0x88: af.hi = adc(af.hi, bc.hi); cycles = 4; break;
            case 0x89: af.hi = adc(af.hi, bc.lo); cycles = 4; break;
            case 0x8A: af.hi = adc(af.hi, de.hi); cycles = 4; break;
            case 0x8B: af.hi = adc(af.hi, de.lo); cycles = 4; break;
            case 0x8C: af.hi = adc(af.hi, hl.hi); cycles = 4; break;
            case 0x8D: af.hi = adc(af.hi, hl.lo); cycles = 4; break;
            case 0x8E: af.hi = adc(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0x8F: af.hi = adc(af.hi, af.hi); cycles = 4; break;
            case 0x90: af.hi = sub(af.hi, bc.hi); cycles = 4; break;
            case 0x91: af.hi = sub(af.hi, bc.lo); cycles = 4; break;
            case 0x92: af.hi = sub(af.hi, de.hi); cycles = 4; break;
            case 0x93: af.hi = sub(af.hi, de.lo); cycles = 4; break;
            case 0x94: af.hi = sub(af.hi, hl.hi); cycles = 4; break;
            case 0x95: af.hi = sub(af.hi, hl.lo); cycles = 4; break;
            case 0x96: af.hi = sub(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0x97: af.hi = sub(af.hi, af.hi); cycles = 4; break;
            case 0x98: af.hi = sbb(af.hi, bc.hi); cycles = 4; break;
            case 0x99: af.hi = sbb(af.hi, bc.lo); cycles = 4; break;
            case 0x9A: af.hi = sbb(af.hi, de.hi); cycles = 4; break;
            case 0x9B: af.hi = sbb(af.hi, de.lo); cycles = 4; break;
            case 0x9C: af.hi = sbb(af.hi, hl.hi); cycles = 4; break;
            case 0x9D: af.hi = sbb(af.hi, hl.lo); cycles = 4; break;
            case 0x9E: af.hi = sbb(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0x9F: af.hi = sbb(af.hi, af.hi); cycles = 4; break;
            case 0xA0: af.hi = ana(af.hi, bc.hi); cycles = 4; break;
            case 0xA1: af.hi = ana(af.hi, bc.lo); cycles = 4; break;
            case 0xA2: af.hi = ana(af.hi, de.hi); cycles = 4; break;
            case 0xA3: af.hi = ana(af.hi, de.lo); cycles = 4; break;
            case 0xA4: af.hi = ana(af.hi, hl.hi); cycles = 4; break;
            case 0xA5: af.hi = ana(af.hi, hl.lo); cycles = 4; break;
            case 0xA6: af.hi = ana(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0xA7: af.hi = ana(af.hi, af.hi); cycles = 4; break;
            case 0xA8: af.hi = xra(af.hi, bc.hi); cycles = 4; break;
            case 0xA9: af.hi = xra(af.hi, bc.lo); cycles = 4; break;
            case 0xAA: af.hi = xra(af.hi, de.hi); cycles = 4; break;
            case 0xAB: af.hi = xra(af.hi, de.lo); cycles = 4; break;
            case 0xAC: af.hi = xra(af.hi, hl.hi); cycles = 4; break;
            case 0xAD: af.hi = xra(af.hi, hl.lo); cycles = 4; break;
            case 0xAE: af.hi = xra(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0xAF: af.hi = xra(af.hi, af.hi); cycles = 4; break;
            case 0xB0: af.hi = ora(af.hi, bc.hi); cycles = 4; break;
            case 0xB1: af.hi = ora(af.hi, bc.lo); cycles = 4; break;
            case 0xB2: af.hi = ora(af.hi, de.hi); cycles = 4; break;
            case 0xB3: af.hi = ora(af.hi, de.lo); cycles = 4; break;
            case 0xB4: af.hi = ora(af.hi, hl.hi); cycles = 4; break;
            case 0xB5: af.hi = ora(af.hi, hl.lo); cycles = 4; break;
            case 0xB6: af.hi = ora(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0xB7: af.hi = ora(af.hi, af.hi); cycles = 4; break;
            case 0xB8: sub(af.hi, bc.hi); cycles = 4; break;
            case 0xB9: sub(af.hi, bc.lo); cycles = 4; break;
            case 0xBA: sub(af.hi, de.hi); cycles = 4; break;
            case 0xBB: sub(af.hi, de.lo); cycles = 4; break;
            case 0xBC: sub(af.hi, hl.hi); cycles = 4; break;
            case 0xBD: sub(af.hi, hl.lo); cycles = 4; break;
            case 0xBE: sub(af.hi, readByte(hl.reg)); cycles = 7; break;
            case 0xBF: sub(af.hi, af.hi); cycles = 4; break;
	    case 0xC0: cycles = retcond(!iszero()); break;
            case 0xC1:
            {
                uint16_t temp = readWord(sp);
                sp += 2;
		bc.reg = temp;
                cycles = 10;
            }
            break;
	    case 0xC2: cycles = jumpcond(getimmword(), !iszero()); break;
            case 0xC3: cycles = jump(getimmword()); break;
            case 0xC4: cycles = callcond(getimmword(), !iszero()); break;
            case 0xC5:
            {
                sp -= 2;
                writeWord(sp, bc.reg);
                cycles = 11;
            }
            break;
            case 0xC6: af.hi = add(af.hi, getimmbyte()); cycles = 7; break;
	    case 0xC7: cycles = (call(0x00) - 6); break;
	    case 0xC8: cycles = retcond(iszero()); break;
            case 0xC9: cycles = ret(); break;
            case 0xCA: cycles = jumpcond(getimmword(), iszero()); break;
            case 0xCB: cycles = jump(getimmword()); break;
            case 0xCC: cycles = callcond(getimmword(), iszero()); break;
            case 0xCD: cycles = call(getimmword()); break;
            case 0xCE: af.hi = adc(af.hi, getimmbyte()); cycles = 7; break;
            case 0xCF: cycles = (call(0x08) - 6); break;
	    case 0xD0: cycles = retcond(!iscarry()); break;
            case 0xD1:
            {
                de.reg = readWord(sp);
                sp += 2;
                cycles = 10;
            }
            break;
	    case 0xD2: cycles = jumpcond(getimmword(), !iscarry()); break;
	    case 0xD3: cycles = outport(); break;
            case 0xD4: cycles = callcond(getimmword(), !iscarry()); break;
            case 0xD5:
            {
                sp -= 2;
                writeWord(sp, de.reg);
                cycles = 11;
            }
            break;
            case 0xD6: af.hi = sub(af.hi, getimmbyte()); cycles = 7; break;
            case 0xD7: cycles = (call(0x10) - 6); break;
	    case 0xD8: cycles = retcond(iscarry()); break;
            case 0xD9: cycles = ret(); break;
            case 0xDA: cycles = jumpcond(getimmword(), iscarry()); break;
	    case 0xDB: cycles = inport(); break;
            case 0xDC: cycles = callcond(getimmword(), iscarry()); break;
            case 0xDD: cycles = call(getimmword()); break;
            case 0xDE: af.hi = sbb(af.hi, getimmbyte()); cycles = 7; break;
            case 0xDF: cycles = (call(0x18) - 6); break;
	    case 0xE0: cycles = retcond(!isparity()); break;
            case 0xE1:
            {
                hl.reg = readWord(sp);
                sp += 2;
                cycles = 10;
            }
            break;
	    case 0xE2: cycles = jumpcond(getimmword(), !isparity()); break;
	    case 0xE3: cycles = xthl(); break;
	    case 0xE4: cycles = jumpcond(getimmword(), !isparity()); break;
            case 0xE5:
            {
                sp -= 2;
                writeWord(sp, hl.reg);
                cycles = 11;
            }
            break;
            case 0xE6: af.hi = ana(af.hi, getimmbyte()); cycles = 7; break;
            case 0xE7: cycles = (call(0x20) - 6); break;
	    case 0xE8: cycles = retcond(isparity()); break;
	    case 0xE9: pc = hl.reg; cycles = 5; break;
	    case 0xEA: cycles = jumpcond(getimmword(), isparity()); break;
	    case 0xEB: cycles = xchg(); break;
            case 0xEC: cycles = callcond(getimmword(), isparity()); break;
            case 0xED: cycles = call(getimmword()); break;
            case 0xEE: af.hi = xra(af.hi, getimmbyte()); cycles = 7; break;
            case 0xEF: cycles = (call(0x28) - 6); break;
	    case 0xF0: cycles = retcond(!issign()); break;
            case 0xF1:
            {
                af.reg = readWord(sp);
                sp += 2;
		af.lo = ((af.lo & 0xD7) | 0x02);
                cycles = 10;
            }
            break;
	    case 0xF2: cycles = jumpcond(getimmword(), !issign()); break;
	    case 0xF3: interruptenabled = false; break;
            case 0xF4: cycles = callcond(getimmword(), !issign()); break;
            case 0xF5:
            {
		sp -= 2;
                writeWord(sp, af.reg);
                cycles = 11;
            }
            break;
            case 0xF6: af.hi = ora(af.hi, getimmbyte()); cycles = 7; break;
            case 0xF7: cycles = (call(0x30) - 6); break;
	    case 0xF8: cycles = retcond(issign()); break;
            case 0xF9: sp = hl.reg; cycles = 5; break;
	    case 0xFA: cycles = jumpcond(getimmword(), issign()); break;
	    case 0xFB: interruptenabled = true; break;
            case 0xFC: cycles = callcond(getimmword(), issign()); break;
            case 0xFD: cycles = call(getimmword()); break;
            case 0xFE: sub(af.hi, getimmbyte()); cycles = 7; break;
            case 0xFF: cycles = (call(0x38) - 6); break;
            default: unrecognizedopcode(opcode); break;
        }

	return cycles;
    }
}
