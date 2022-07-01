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

#ifndef BERRN_K052109_VIDEO_H
#define BERRN_K052109_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
#include <konami/k052109.h>
#include <konami/k051962.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    using k052109callback = function<uint32_t(int, uint8_t, uint8_t, int)>;
    class k052109video
    {
	public:
	    k052109video(berrndriver &drv);
	    ~k052109video();

	    void init();
	    void setROM(vector<uint8_t> &tile_rom);
	    void shutdown();
	    void startLog(string filename);
	    void stopLog();
	    void setCallback(k052109callback cb);

	    bool isIRQEnabled();
	    bool getRMRD();

	    uint32_t create_tilemap_addr(uint32_t tile_num, uint8_t color_attrib);

	    array<int, (512 * 256)> render(int layer);

	    void logRender();

	    uint8_t read(uint16_t addr);
	    void write(uint16_t addr, uint8_t data);

	    uint16_t read16(bool upper, bool lower, uint32_t addr);
	    void write16(bool upper, bool lower, uint32_t addr, uint16_t data);

	    void setRMRD(bool line);

	    array<uint8_t, 0x4000> getVRAM()
	    {
		return tile_chip.get_vram();
	    }

	private:
	    berrndriver &driver;
	    ofstream log_file;

	    K052109 tile_chip;
	    K051962 gfx_chip;

	    vector<uint8_t> tile_rom;

	    k052109callback tilecb;

	    void open_file(string filename)
	    {
		log_file.open(filename, ios::out | ios::binary | ios::trunc);
	    }

	    void close_file()
	    {
		if (log_file.is_open())
		{
		    log_file.close();
		}
	    }

	    void write_byte(uint8_t data)
	    {
		if (!log_file.is_open())
		{
		    return;
		}

		log_file.write((char*)&data, 1);
	    }

	    void write_word(uint16_t data)
	    {
		write_byte(data >> 8);
		write_byte(data & 0xFF);
	    }

	    void write_long(uint32_t data)
	    {
		write_word(data >> 16);
		write_word(data & 0xFFFF);
	    }

	    void write_vector(vector<uint8_t> data)
	    {
		write_long(data.size());

		for (size_t index = 0; index < data.size(); index++)
		{
		    write_byte(data.at(index));
		}
	    }

	    void log_rmrd(bool is_line)
	    {
		// Command 0x01: Assert K052109 RM/RD line
		// Command 0x02: Clear K052109 RM/RD line
		uint8_t cmd = (is_line) ? 0x01 : 0x02;
		write_byte(cmd);
	    }

	    void log_render()
	    {
		// Command 0x03: Render video frame
		write_byte(0x03);
	    }

	    void log_write(uint16_t address, uint8_t data)
	    {
		// Command 0x00: K052109/K051962 write
		write_byte(0x00);
		write_word(address);
		write_byte(data);
	    }

	    void log_end()
	    {
		// Command 0x04: End of log
		write_byte(0x04);
	    }

	    void write_header()
	    {
		// ID tag: BEEKONAMIDMP in ASCII
		vector<uint8_t> header_data = {0x42, 0x45, 0x45, 0x4B, 0x4F, 0x4E, 0x41, 0x4D, 0x49, 0x44, 0x4D, 0x50};

		for (size_t index = 0; index < header_data.size(); index++)
		{
		    write_byte(header_data.at(index));
		}

		// Major version (should be 1)
		write_byte(0x01);
		// Minor version (should be 0)
		write_byte(0x00);

		// Bit 0 = K052109/K051962 enabled
		// Bits 1-7 = Reserved
		write_byte(0x01);

		// Write raw tile ROM to log
		write_byte(0x05);
		write_vector(tile_rom);

		// Write snapshot of VRAM to log
		auto vram = tile_chip.get_vram();
		vector<uint8_t> vram_vec = vector<uint8_t>(vram.begin(), vram.end());

		write_byte(0x06);
		write_vector(vram_vec);
	    }
    };
};

#endif // BERRN_K052109_VIDEO_H