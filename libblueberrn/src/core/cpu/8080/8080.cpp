#include "8080.h"
#include <cstdlib>
#include <iostream>
using namespace intel8080;
using namespace std;

namespace intel8080
{
    void i8080::reset()
    {
        af.reg = 0;
        bc.reg = 0;
        de.reg = 0;
        hl.reg = 0;
        sp = 0;
        pc = 0;

	af.lo = BitReset(af.lo, 5);
	af.lo = BitReset(af.lo, 3);
	af.lo = BitSet(af.lo, 1);
        
        cout << "Intel 8080::Initialized" << endl;
    }
    
    void i8080::shutdown()
    {
        cout << "Intel 8080::Shutting down..." << endl;
    }

    void i8080::setmemcallback(i8080readb readcb, i8080writeb writecb)
    {
	readByte = readcb;
	writeByte = writecb;
    }

    void i8080::setportcallback(i8080infunc incb, i8080outfunc outcb)
    {
	readPort = incb;	
	writePort = outcb;
    }
    
    void i8080::executenextopcode()
    {
        uint8_t opcode = readByte(pc++);
        executeopcode(opcode);
    }

    void i8080::printregs()
    {
	cout << "AF: " << hex << (int)(af.reg)<< endl;
	cout << "BC: " << hex << (int)(bc.reg) << endl;
	cout << "DE: " << hex << (int)(de.reg) << endl;
	cout << "HL: " << hex << (int)(hl.reg) << endl;
	cout << "PC: " << hex << (int)(pc) << endl;
	cout << "SP: " << sp << endl;
	cout << "Opcode: " << hex << (int)(readByte(pc)) << endl;
	cout << "Interrupt enabled: " << (int)(interruptenabled) << endl;
	cout << "Flags: ";
	cout << (issign() ? "S" : ".");
	cout << (iszero() ? "Z" : ".");
	cout << ".";
	cout << (ishalf() ? "A" : ".");
	cout << ".";
	cout << (isparity() ? "P" : ".");
	cout << ".";
	cout << (iscarry() ? "C" : ".") << endl;
	cout << endl;
    }
    
    void i8080::unrecognizedopcode(uint8_t opcode)
    {       
	cout << "Unrecognized opcode at 0x" << hex << (int)opcode << endl;
        exit(1);
    }

    void i8080::generateinterrupt(int num)
    {
	if (interruptenabled)
	{
	    sp -= 2;
	    writeWord(sp, pc);
	    pc = (8 * num);
	    interruptgenerated = true;
	    interruptenabled = false;
	}
    }

    int i8080::runfor(int cycles)
    {
	while (cycles > 0)
	{

	    if (interruptgenerated)
	    {
		cycles -= 11;
		interruptgenerated = false;
	    }

	    cycles -= executeopcode(readByte(pc++));
	}

	return cycles;
    }
};
