#ifndef LIBBLUEBERRN_MB14241_SHIFTER_H
#define LIBBLUEBERRN_MB14241_SHIFTER_H

#include <cstdint>
#include <iostream>
using namespace std;

namespace mb14241
{
    struct mb14241shifter
    {
	int shiftoffs = 0;
	uint16_t shiftval = 0;

	void setshiftoffs(uint8_t val)
	{
	    shiftoffs = (val & 0x07);
	}

	void fillshiftreg(uint8_t val)
	{
	    shiftval >>= 8;
	    shiftval |= (val << 8);
	}

	uint8_t readshiftresult()
	{
	    return (shiftval >> (8 - shiftoffs));
	}
    };
};




#endif // LIBBLUEBERRN_MB14241_SHIFTER_H