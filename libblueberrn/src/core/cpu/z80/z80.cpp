#include "z80.h"
#include <cstdlib>
#include <iostream>
using namespace zilogz80;
using namespace std;

namespace zilogz80
{   
    void z80::reset()
    {
        af.reg = 0;
        bc.reg = 0;
        de.reg = 0;
        hl.reg = 0;
        afs.reg = 0;
        bcs.reg = 0;
        des.reg = 0;
        hls.reg = 0;
        ix.reg = 0;
        iy.reg = 0;
        sp = 0;
        pc = 0;
        
        cout << "Zilog Z80::Initialized" << endl;
    }
    
    void z80::shutdown()
    {
        cout << "Zilog Z80::Shutting down..." << endl;
    }

    void z80::printregs()
    {
	cout << "AF: " << hex << (int)(af.reg)<< endl;
	cout << "BC: " << hex << (int)(bc.reg) << endl;
	cout << "DE: " << hex << (int)(de.reg) << endl;
	cout << "HL: " << hex << (int)(hl.reg) << endl;
	cout << "PC: " << hex << (int)(pc) << endl;
	cout << "SP: " << sp << endl;
	cout << "Opcode: " << hex << (int)(readByte(pc)) << endl;
	cout << endl;
    }
    
    void z80::executenextopcode()
    {
        uint8_t opcode = readByte(pc++);
        executeopcode(opcode);
    }
    
    void z80::unrecognizedopcode(uint8_t opcode)
    {
        cout << "Unrecognized opcode at 0x" << hex << (int)opcode << endl;
        exit(1);
    }

    int z80::runfor(int cycles)
    {
	while (cycles > 0)
	{
	    printregs();
	    cycles -= executeopcode(readByte(pc++));
	}

	return cycles;
    }

    void z80::setmemcallback(z80readb readcb, z80writeb writecb)
    {
	readByte = readcb;
	writeByte = writecb;
    }
};
