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

#ifndef BERRN_KONMEDAL_VIDEO_H
#define BERRN_KONMEDAL_VIDEO_H

#include <libblueberrn_api.h>
#include <utils.h>
#include <graphics.h>
#include <driver.h>
#include <video/k052109.h>
using namespace berrn;
using namespace std;

namespace berrn
{
    class shuriboyvideo
    {
	public:
	    shuriboyvideo(berrndriver &drv);
	    ~shuriboyvideo();

	    void init();
	    void shutdown();

	    void setRMRD(bool line);
	    uint8_t read(uint16_t addr);
	    void write(uint16_t addr, uint8_t data);

	    void dump();

	    void updatePixels();

	protected:
	    virtual void initLayerOrder();

	    BerrnBitmapRGB *bitmap = NULL;
	    array<berrnRGBA, 256> colors;

	    k052109video *tilemap = NULL;

	    void setLayerOrder(int layer1, int layer2, int layer3)
	    {
		layer_order[0] = layer1;
		layer_order[1] = layer2;
		layer_order[2] = layer3;
	    }

	private:
	    berrndriver &driver;

	    vector<uint8_t> tile_rom;

	    void render(int layer, int priority);

	    array<int, 3> layer_order = {0, 0, 0};

	    int frame_number = 0;
	    bool is_dump_active = false;
    };

    class mariorouvideo : public shuriboyvideo
    {
	public:
	    mariorouvideo(berrndriver &drv);
	    ~mariorouvideo();

	    void initLayerOrder();
    };
};

#endif // BERRN_KONMEDAL_VIDEO_H