#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
using namespace std;

#if defined(MSB_FIRST) || defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define IS_BIG_ENDIAN
#else
#define IS_LITTLE_ENDIAN
#endif

namespace intel8080
{
    using i8080readb = function<uint8_t(uint16_t)>;
    using i8080writeb = function<void(uint16_t, uint8_t)>;
    using i8080infunc = function<uint8_t(uint8_t)>;
    using i8080outfunc = function<void(uint8_t, uint8_t)>;

    union Register
    {
        struct
        {
            #ifdef IS_LITTLE_ENDIAN
            	uint8_t lo;
		uint8_t hi;
	    #else
	        uint8_t hi;
		uint8_t lo;
	    #endif
	};
	uint16_t reg;
    };
    
    struct i8080
    {   
        void reset();
        void shutdown();
        void executenextopcode();
        int executeopcode(uint8_t opcode);
	int runfor(int cycles);
        void unrecognizedopcode(uint8_t opcode);
	void printregs();
            
        Register af;
        Register bc;
        Register de;
        Register hl;
        uint16_t sp;
        uint16_t pc;
            
        int mcycles = 0;
        int sign = 7;
        int zero = 6;
        int half = 4;
        int parity = 2;
        int carry = 0;

	bool interruptenabled = false;
	bool interruptgenerated = false;
	void generateinterrupt(int num);
            
        i8080readb readByte;
        i8080writeb writeByte;
	i8080infunc readPort;
	i8080outfunc writePort;

        uint16_t readWord(uint16_t address)
        {
            return (readByte(address + 1) << 8) | readByte(address);
        }
        
        void writeWord(uint16_t address, uint16_t value)
        {
            writeByte(address, (value & 0xFF));
            writeByte((address + 1), (value >> 8));
        }

	int inport()
	{
	    uint8_t addr = readByte(pc++);
	    af.hi = readPort(addr);
	    return 10;
	}

	int outport()
	{
	    uint8_t addr = readByte(pc++);
	    writePort(addr, af.hi);
	    return 10;
	}

	void setmemcallback(i8080readb readcb, i8080writeb writecb);
	void setportcallback(i8080infunc incb, i8080outfunc outcb);
            
        bool TestBit(uint8_t value, int bit)
        {
            return (value & (1 << bit)) ? true : false;
        }
            
        uint8_t BitSet(uint8_t value, int bit)
        {
            return (value | (1 << bit));
        }
            
        uint8_t BitReset(uint8_t value, int bit)
        {
            return (value & ~(1 << bit));
        }
            
        int BitGetVal(uint8_t value, int bit)
        {
            return (value & (1 << bit)) ? 1 : 0;
        }

	bool determineparity(uint8_t reg)
	{
	    uint8_t nbonebits = 0;
	    for (int i = 0; i < 8; i++)
	    {
		nbonebits += ((reg >> i) & 1);
	    }

	    return ((nbonebits & 1) == 0);
	}

	bool issign()
	{
	    return TestBit(af.lo, 7);
	}

	void setsign(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 7) : BitReset(af.lo, 7);
	}

	bool iszero()
	{
	    return TestBit(af.lo, 6);
	}

	void setzero(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 6) : BitReset(af.lo, 6);
	}

	bool ishalf()
	{
	    return TestBit(af.lo, 4);
	}

	void sethalf(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 4) : BitReset(af.lo, 4);
	}

	bool isparity()
	{
	    return TestBit(af.lo, 2);
	}

	void setparity(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 2) : BitReset(af.lo, 2);
	}

	bool iscarry()
	{
	    return TestBit(af.lo, 0);
	}

	void setcarry(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 0) : BitReset(af.lo, 0);
	}

	uint8_t getimmbyte()
	{
	    uint8_t temp = readByte(pc++);
	    return temp;
	};

	uint16_t getimmword()
	{
	    uint16_t temp = readWord(pc);
	    pc += 2;
	    return temp;
	}
            
        uint8_t add(uint8_t reg1, uint8_t reg2)
        {
            uint8_t result = (reg1 + reg2);
            
	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    sethalf((((reg1 + reg2) ^ reg1 ^ reg2)) & 0x10);
	    setcarry(((reg1 + reg2) > 0xFF));
	    setparity(determineparity((result & 0xFF)));
                
            return (reg1 + reg2);
        }

	uint16_t addreg(uint16_t reg1, uint16_t reg2)
	{
	    setcarry((reg1 + reg2) > 0xFFFF);

	    return (reg1 + reg2);
	}
            
        uint8_t adc(uint8_t reg1, uint8_t reg2)
        {
            uint8_t temp = reg2;
                
            if (iscarry())
            {
                temp += 1;
            }
                
            uint8_t result = (reg1 + temp);

	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    sethalf(((reg1 & 0x0F) + (temp & 0x0F)) > 0x0F);
	    setcarry(((reg1 + temp) > 0xFF));
	    setparity(determineparity((result & 0xFF)));
                
            return (reg1 + temp);
        }
            
        uint8_t sub(uint8_t reg1, uint8_t reg2)
        {
            uint8_t result = (reg1 - reg2);

	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    sethalf(~(reg1 ^ reg2 ^ result) & 0x10);
	    setcarry((reg1 < reg2));
	    setparity(determineparity((result & 0xFF)));
                
            return (reg1 - reg2);
        }
            
        uint8_t sbb(uint8_t reg1, uint8_t reg2)
        {
            uint8_t temp = reg2;
                
            if (iscarry())
            {
                temp += 1;
            }
                
            uint8_t result = (reg1 - temp);

	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    sethalf(~(reg1 ^ temp ^ result) & 0x10);
	    setcarry((reg1 < temp));
	    setparity(determineparity((result & 0xFF)));
                
            return (reg1 - temp);
        }
        
        uint8_t ana(uint8_t reg1, uint8_t reg2)
        {
            uint8_t result = (reg1 & reg2);

	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    setcarry(false);
	    setparity(determineparity((result & 0xFF)));
	    sethalf((reg1 | reg2) & 0x08);
            
            return (reg1 & reg2);
        }
        
        uint8_t ora(uint8_t reg1, uint8_t reg2)
        {
            uint8_t result = (reg1 | reg2);
            
	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    setcarry(false);
	    setparity(determineparity((result & 0xFF)));
	    sethalf(false);
            
            return (reg1 | reg2);
        }
        
        uint8_t xra(uint8_t reg1, uint8_t reg2)
        {
            uint8_t result = (reg1 ^ reg2);
            
	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    setcarry(false);
	    setparity(determineparity((result & 0xFF)));
	    sethalf(false);
            
            return (reg1 ^ reg2);
        }

	uint8_t inr(uint8_t reg)
	{
	    uint8_t result = (reg + 1);

	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    setparity(determineparity((result & 0xFF)));
	    sethalf((reg & 0x0F) == 0x0F);

	    return (reg + 1);
	}

	uint8_t dcr(uint8_t reg)
	{
	    uint8_t result = (reg - 1);

	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    setparity(determineparity((result & 0xFF)));
	    sethalf((reg & 0x0F) != 0x00);

	    return (reg - 1);
	}

	void daa()
	{
	    uint8_t temp = 0;

	    if (((af.hi & 0xF) > 9) || (ishalf()))
	    {
		temp |= 0x06;
	    }

	    bool carry = iscarry();


	    if (((af.hi & 0xF0) > 0x90) || (((af.hi & 0xF0) >= 0x90) && ((af.hi & 0xF) > 9)) || iscarry())
	    {
		temp |= 0x60;
		carry = true;
	    }

	    af.hi = add(af.hi, temp);

	    setcarry(carry);
	}

	void rlcaccum()
	{
	    setcarry((TestBit(af.hi, 7)));
	    af.hi = ((af.hi << 1) | (iscarry()));
	}

	void rrcaccum()
	{
	    setcarry((TestBit(af.hi, 0)));
	    af.hi = ((af.hi >> 1) | (iscarry() << 7));
	}

	void ralaccum()
	{
	    bool carry = iscarry();
	    setcarry((TestBit(af.hi, 7)));
	    af.hi = ((af.hi << 1) | carry);
	}

	void raraccum()
	{
	    bool carry = iscarry();
	    setcarry((TestBit(af.hi, 0)));
	    af.hi = ((af.hi >> 1) | (carry << 7));
	}

	int jump(uint16_t addr)
	{
	    pc = addr;
	    return 10;
	}

	int jumpcond(uint16_t addr, bool cond)
	{
	    if (cond)
	    {
		return jump(addr);
	    }
	    else
	    {
		return 10;
	    }
	}

	int call(uint16_t addr)
	{
	    sp -= 2;
            writeWord(sp, pc);
            pc = addr;
            return 17;
	}

	int callcond(uint16_t addr, bool cond)
	{
	    if (cond)
	    {
		return call(addr);
	    }
	    else
	    {
		return 11;
	    }
	}

	int ret()
	{
	    uint16_t temp = readWord(sp);                
	    sp += 2;
            pc = temp;
            return 10;
	}

	int retcond(bool cond)
	{
	    if (cond)
	    {
		return (1 + ret()); // 11 cycles
	    }
	    else
	    {
		return 5;
	    }
	}

	int xthl()
	{
	    uint16_t temp = readWord(sp);
	    writeWord(sp, hl.reg);
	    hl.reg = temp;
	    return 18;
	}

	int xchg()
	{
	    uint16_t temp = de.reg;
	    de.reg = hl.reg;
	    hl.reg = temp;
	    return 5;
	}
            
        int bitcount(uint8_t value)
        {
            uint8_t temp = value;
                
            int count = 0;
            while (temp)
            {
                count += (temp & 1);
                temp >>= 1;
            }
                
            return count;
        }
    };
};
