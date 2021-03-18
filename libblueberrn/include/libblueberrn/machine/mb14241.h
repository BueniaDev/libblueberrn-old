#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
using namespace std;

namespace mb14241
{
    struct mb14241shifter
    {
	uint8_t shiftcount = 0;
	uint16_t shiftdata = 0;

	inline void setshiftoffset(uint8_t value)
	{
	    shiftcount = (~value & 0x07);
	}

	inline void fillshiftreg(uint8_t value)
	{
	    shiftdata = ((shiftdata >> 8) | ((uint16_t)(value) << 7));
	}

	inline uint8_t getresult()
	{
	    return (shiftdata >> shiftcount);
	}
    };
};
