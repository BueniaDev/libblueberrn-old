#include <cstdint>
#include <functional>
using namespace std;

#if defined(MSB_FIRST) || defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define IS_BIG_ENDIAN
#else
#define IS_LITTLE_ENDIAN
#endif

namespace zilogz80
{
    typedef function<uint8_t(uint16_t)> z80readb;
    typedef function<void(uint16_t, uint8_t)> z80writeb;

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
    
    struct z80
    {
        void reset();
        void shutdown();
	int runfor(int cycles);
        void executenextopcode();
        int executeopcode(uint8_t opcode);
        void unrecognizedopcode(uint8_t opcode);
	void printregs();
            
        Register af;
        Register bc;
        Register de;
        Register hl;
        Register afs;
        Register bcs;
        Register des;
        Register hls;
        Register ix;
        Register iy;
        uint16_t sp;
        uint16_t pc;
            
        int mcycles = 0;
            
        z80readb readByte;
        z80writeb writeByte;
        
        uint16_t readWord(uint16_t address)
        {
            return (readByte(address + 1) << 8) | readByte(address);
        }
        
        void writeWord(uint16_t address, uint16_t value)
        {
            writeByte(address, (value & 0xFF));
            writeByte((address + 1), (value >> 8));
        }

	void setmemcallback(z80readb readcb, z80writeb writecb);
            
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

	inline bool issign()
	{
	    return TestBit(af.lo, 7);
	}

	inline void setsign(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 7) : BitReset(af.lo, 7);
	}

	inline bool iszero()
	{
	    return TestBit(af.lo, 6);
	}

	inline void setzero(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 6) : BitReset(af.lo, 6);
	}

	inline bool ishalf()
	{
	    return TestBit(af.lo, 4);
	}

	inline void sethalf(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 4) : BitReset(af.lo, 4);
	}

	inline bool ispariflow()
	{
	    return TestBit(af.lo, 2);
	}

	inline void setpariflow(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 2) : BitReset(af.lo, 2);
	}

	inline bool issubtract()
	{
	    return TestBit(af.lo, 1);
	}

	inline void setsubtract(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 1) : BitReset(af.lo, 1);
	}

	inline bool iscarry()
	{
	    return TestBit(af.lo, 0);
	}

	inline void setcarry(bool val)
	{
	    af.lo = (val) ? BitSet(af.lo, 0) : BitReset(af.lo, 0);
	}

	uint8_t getimmbyte()
	{
	    uint8_t temp = readByte(pc++);
	    return temp;
	}

	int8_t getimmsignedbyte()
	{
	    int8_t temp = (int8_t)(getimmbyte());
	    return temp;
	}

	uint16_t getimmword()
	{
	    uint16_t temp = readWord(pc);
	    pc += 2;
	    return temp;
	}

        uint8_t addreg(uint8_t reg1, uint8_t reg2)
        {
            uint8_t result = (reg1 + reg2);
            int8_t overtemp = (int8_t)(reg1 + reg2);
	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    setsubtract(false);
	    sethalf((((reg1 + reg2) ^ reg1 ^ reg2)) & 0x10);
	    setcarry(((reg1 + reg2) > 0xFF));
	    setpariflow((overtemp > 127) || (overtemp < -128));
                
            return (reg1 + reg2);
        }

	uint8_t xorreg(uint8_t reg1, uint8_t reg2)
	{
	    uint8_t result = (reg1 ^ reg2);
	    setcarry(false);
	    sethalf(false);
	    setsubtract(false);
	    setzero((result == 0));
	    setsign(TestBit(result, 7));
	    setpariflow(determineparity(result));

	    return (reg1 ^ reg2);
	}

	uint8_t increg(uint8_t reg)
	{
	    uint8_t result = (reg + 1);
	    int8_t overtemp = (int8_t)((reg + 1));
	    setsubtract(false);
	    setzero((result == 0));
	    setsign(TestBit(result, 7));	    
	    sethalf((reg & 0x0F) == 0x0F);
	    setpariflow((overtemp > 127) || (overtemp < -128));

	    return (reg + 1);
	}

	uint8_t decreg(uint8_t reg)
	{
	    uint8_t result = (reg - 1);
	    int8_t overtemp = (int8_t)((reg - 1));
	    setsubtract(true);
	    setzero((result == 0));
	    setsign(TestBit(result, 7));	    
	    sethalf((reg & 0x0F) != 0x00);
	    setpariflow((overtemp > 127) || (overtemp < -128));

	    return (reg - 1);
	}

	int djnz(int8_t addr)
	{
	    bc.hi -= 1;
	    
	    if (bc.hi != 0)
	    {
		pc += addr;
		return 13;
	    }
	    else
	    {
		return 8;
	    }
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

	int exchangedehl()
	{
	    uint16_t temp = de.reg;
	    de.reg = hl.reg;
	    hl.reg = temp;
	    return 4;
	}
    };
};
