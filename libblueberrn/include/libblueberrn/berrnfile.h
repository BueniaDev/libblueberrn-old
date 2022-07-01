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

#ifndef BERRN_FILE_H
#define BERRN_FILE_H

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <vector>
using namespace std;

namespace berrn
{
    class BerrnFile
    {
	public:
	    BerrnFile() : file_pos(0)
	    {

	    }

	    void open(vector<uint8_t> buffer)
	    {
		bytes = vector<uint8_t>(buffer);
	    }

	    void close()
	    {
		file_pos = 0;
		bytes.clear();
	    }

	    uint8_t read_u8()
	    {
		if (!has_bytes(1))
		{
		    return 0xFF;
		}

		uint8_t byte_val = bytes.at(file_pos);
		file_pos += 1;
		return byte_val;
	    }

	    uint16_t read_u16_le()
	    {
		uint8_t data[2];

		for (int i = 0; i < 2; i++)
		{
		    data[i] = read_u8();
		}

		return ((data[1] << 8) | data[0]);
	    }

	    int16_t read_i16_le()
	    {
		return int16_t(read_u16_le());
	    }

	    uint32_t read_u32_le()
	    {
		uint8_t data[4];

		for (int i = 0; i < 4; i++)
		{
		    data[i] = read_u8();
		}

		return ((data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0]);
	    }

	    string read_fourcc()
	    {
		string fourcc;
		for (int i = 0; i < 4; i++)
		{
		    fourcc.push_back(read_u8());
		}

		return fourcc;
	    }

	    void advance_bytes(size_t num_bytes)
	    {
		file_pos += num_bytes;
	    }

	    bool has_ended()
	    {
		return (file_pos > bytes.size());
	    }

	private:
	    vector<uint8_t> bytes;
	    size_t file_pos = 0;

	    bool has_bytes(size_t num_bytes)
	    {
		if ((bytes.size() - file_pos) >= num_bytes)
		{
		    return true;
		}

		return false;
	    }
    };
};



#endif // BERRN_FILE_H