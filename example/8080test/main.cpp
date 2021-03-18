#include "../../libblueberrn/src/core/cpu/8080/8080.h"
#include <iostream>
#include <functional>
#include <fstream>
using namespace intel8080;
using namespace std;
using namespace std::placeholders;

uint8_t memory[0x10000];

i8080 cpu8080;

bool loadfile(string filename)
{
    ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

    if (file.is_open())
    {
	streampos size = file.tellg();
	file.seekg(0, ios::beg);
	file.read((char*)&memory[0x100], size);
	file.close();
	cout << "Success" << endl;
	return true;
    }
    else
    {
	cout << "Error" << endl;
	return false;
    }
}

uint8_t readb(uint16_t address)
{
    return memory[address];
}

void writeb(uint16_t address, uint8_t value)
{
    memory[address] = value;
}

uint8_t readport(uint8_t address)
{
    return 0;
}

void writeport(uint8_t address, uint8_t value)
{
    return;
}

void init()
{
    auto rb = bind(readb, _1);
    auto wb = bind(writeb, _1, _2);
    auto rp = bind(readport, _1);
    auto wp = bind(writeport, _1, _2);
    cpu8080.setmemcallback(rb, wb);
    cpu8080.setportcallback(rp, wp);
}

void interceptbdoscall()
{
    if (cpu8080.bc.lo == 2)
    {
	if (cpu8080.de.lo != 0)
	{
	    putchar((char)cpu8080.de.lo);
	}
    }
    else if (cpu8080.bc.lo == 9)
    {
	for (int addr = cpu8080.de.reg; cpu8080.readByte(addr) != '$'; addr++)
	{
	    if (cpu8080.readByte(addr) != 0)
	    {
		putchar((char)cpu8080.readByte(addr));
	    }
	}
    }
}

int main(int argc, char* argv[])
{
    init();

    cpu8080.reset();
    cpu8080.pc = 0x100;

    if (!loadfile(argv[1]))
    {
	return 1;
    }

    cpu8080.writeByte(5, 0xC9);

    while (true)
    {
	cpu8080.executenextopcode();

	if (cpu8080.pc == 0)
	{
	    cout << "Success" << endl;
	    return 0;
	}

	if (cpu8080.pc == 5)
	{
	    interceptbdoscall();
	}
    }

    return 0;
}
