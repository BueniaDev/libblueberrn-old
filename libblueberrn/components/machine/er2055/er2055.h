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

#ifndef LIBBLUEBERRN_ER2055_IMPL_H
#define LIBBLUEBERRN_ER2055_IMPL_H

namespace er2055
{
    class er2055core
    {
	public:
	    er2055core()
	    {
		earom_data.fill(0xFF);
	    }

	    ~er2055core()
	    {

	    }

	    uint8_t data()
	    {
		return rom_data;
	    }

	    void set_address(uint8_t addr)
	    {
		rom_addr = (addr & 0x3F);
	    }

	    void set_data(uint8_t data)
	    {
		rom_data = data;
	    }

	    void set_control(bool cs1, bool cs2, bool c1, bool c2)
	    {
		uint8_t old_state = get_control();

		is_cs1 = cs1;
		is_cs2 = cs2;
		is_c1 = c1;
		is_c2 = c2;

		uint8_t control_state = get_control();

		if ((!is_cs1 || !is_cs2) || (control_state == old_state))
		{
		    return;
		}

		update_state();
	    }

	    void set_clk(bool line)
	    {
		uint8_t old_state = get_control();

		is_clk = line;

		uint8_t control_state = get_control();

		// Updates occur on the falling edge of the clock
		// when the chip is selected
		if ((is_cs1 && is_cs2) && (control_state != old_state) && !line)
		{
		    // Read mode (C2 is "don't care")
		    if (is_c1)
		    {
			rom_data = earom_data.at(rom_addr);
		    }

		    update_state();
		}
	    }

	private:
	    bool is_clk = false;
	    bool is_c1 = false;
	    bool is_c2 = false;
	    bool is_cs1 = false;
	    bool is_cs2 = false;

	    int rom_addr = 0;
	    uint8_t rom_data = 0;

	    array<uint8_t, 0x40> earom_data;

	    void update_state()
	    {
		// Write mode
		// Note: Erasing is required, so we perform an AND against
		// the previous data to simulate the incorrect behavior
		// if the erasing was not done
		if (!is_c1 && !is_c2)
		{
		    cout << "Writing value of " << hex << int(rom_data) << " to EAROM address of " << hex << int(rom_addr) << endl;
		    earom_data.at(rom_addr) &= rom_data;
		}
		// Erase mode
		else if (is_c2)
		{
		    cout << "Erasing EAROM value at address of " << hex << int(rom_addr) << endl;
		    earom_data.at(rom_addr) = 0xFF;
		}
	    }

	    uint8_t get_control()
	    {
		uint8_t data = is_clk;
		data |= (is_c1 << 1);
		data |= (is_c2 << 2);
		data |= (is_cs1 << 3);
		data |= (is_cs2 << 4);
		
		return data;
	    }
    };
};

#endif // LIBBLUEBERRN_ER2055_IMPL_H