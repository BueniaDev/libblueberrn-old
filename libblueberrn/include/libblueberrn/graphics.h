/*
    This file is part of libblueberrn.
    Copyright (C) 2021 BueniaDev.

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

#ifndef BERRN_GFX_H
#define BERRN_GFX_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <utils.h>
#include <libblueberrn_api.h>
using namespace std;

namespace berrn
{
    struct berrnRGBA
    {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
    };
	
    inline berrnRGBA red()
    {
	return {255, 0, 0, 255};
    }
	
    inline berrnRGBA green()
    {
	return {0, 255, 0, 255};
    }
	
    inline berrnRGBA blue()
    {
	return {0, 0, 255, 255};
    }

    inline berrnRGBA cyan()
    {
	return {0, 255, 255, 255};
    }

    inline berrnRGBA magenta()
    {
	return {255, 0, 255, 255};
    }

    inline berrnRGBA yellow()
    {
	return {255, 255, 0, 255};
    }

    inline berrnRGBA black()
    {
	return {0, 0, 0, 255};
    }

    inline berrnRGBA white()
    {
	return {255, 255, 255, 255};
    }

    inline berrnRGBA fromColor(uint8_t color)
    {
	return {color, color, color, 255};
    }

    inline berrnRGBA fromRGB(uint8_t red, uint8_t green, uint8_t blue)
    {
	return {red, green, blue, 255};
    }

    inline berrnRGBA fromRGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    {
	return {red, green, blue, alpha};
    }

    enum BerrnBitmapFormat
    {
	BerrnRGB
    };

    class BerrnBitmap
    {
	public:
	    virtual ~BerrnBitmap()
	    {

	    }

	    int width() const
	    {
		return width_;
	    }

	    int height() const
	    {
		return height_;
	    }

	    BerrnBitmapFormat format() const
	    {
		return format_;
	    }

	    virtual berrnRGBA pixel(int x, int y) = 0;
	    virtual void setPixel(int x, int y, berrnRGBA color) = 0;
	    virtual void clear() = 0;

	protected:
	    BerrnBitmap(int width, int height, BerrnBitmapFormat format) :
		width_(width), height_(height), format_(format)
	    {

	    }

	private:
	    int width_;
	    int height_;
	    BerrnBitmapFormat format_;
    };

    class BerrnBitmapRGB : public BerrnBitmap
    {
	public:
	    BerrnBitmapRGB(int width, int height) : BerrnBitmap(width, height, BerrnRGB)
	    {
		framebuffer.resize((width * height), black());
	    }

	    berrnRGBA pixel(int x, int y)
	    {
		berrnRGBA color = black();

		if (inRange(x, 0, width()) && inRange(y, 0, height()))
		{
		    color = framebuffer[x + (y * width())];
		}

		return color;
	    }

	    void setPixel(int x, int y, berrnRGBA color)
	    {
		if (inRange(x, 0, width()) && inRange(y, 0, height()))
		{
		    framebuffer[x + (y * width())] = color;
		}
	    }

	    void clear()
	    {
		fill(framebuffer.begin(), framebuffer.end(), black());
	    }

	    void fillcolor(berrnRGBA color)
	    {
		fill(framebuffer.begin(), framebuffer.end(), color);
	    }

	    vector<berrnRGBA> data() const
	    {
		return framebuffer;
	    }

	    int pitch() const
	    {
		return (width() * sizeof(berrnRGBA));
	    }

	private:
	    vector<berrnRGBA> framebuffer;
    };

    #define gfx_step2(start, step) start, (start + step)
    #define gfx_step4(start, step) gfx_step2(start, step), gfx_step2((start + (2 * step)), step)
    #define gfx_step8(start, step) gfx_step4(start, step), gfx_step4((start + (4 * step)), step)

    #define gfx_rstep2(start, step) (start + step), start
    #define gfx_rstep4(start, step) gfx_rstep2((start + (2 * step)), step), gfx_rstep2(start, step)
    #define gfx_rstep8(start, step) gfx_rstep4((start + (4 * step)), step), gfx_rstep4(start, step)

    struct BerrnGfxLayout
    {
	int width; // Width of a tile (in pixels)
	int height; // Height of a tile (in pixels)
	int num_tiles; // Number of total tiles
	int num_planes; // Number of bit planes
	array<int, 4> plane_offs; // Offset of bit planes (in bits)
	array<int, 16> x_offs; // Offset of pixel x-coordinate in bits relative to one bitplane (in bits)
	array<int, 16> y_offs; // Offset of pixel y-coordinate in bits relative to one bitplane (in bits)
	int delta; // Length of an individual tile (in bits)
    };

    inline void gfxDecodeChar(BerrnGfxLayout &layout, vector<uint8_t> src, vector<uint8_t> &dst, int offset_bits, int dst_offs)
    {
	for (int ypos = 0; ypos < layout.height; ypos++)
	{
	    for (int xpos = 0; xpos < layout.width; xpos++)
	    {
		uint32_t base_offs = (layout.y_offs[ypos] + layout.x_offs[xpos] + offset_bits);

		uint8_t video_byte = 0;

		for (int plane = 0; plane < layout.num_planes; plane++)
		{
		    int plane_bit = ((layout.num_planes - 1) - plane);
		    uint32_t bit_offs = (base_offs + layout.plane_offs[plane]);

		    uint32_t byte_num = (bit_offs / 8);
		    int bit_num = (7 - (bit_offs % 8));

		    if (testbit(src[byte_num], bit_num))
		    {
			video_byte = setbit(video_byte, plane_bit);
		    }
		}

		int index = (xpos + (ypos * layout.width));
		dst[dst_offs + index] = video_byte;
	    }
	}
    }

    inline void gfxDecodeSet(BerrnGfxLayout &layout, vector<uint8_t> src, vector<uint8_t> &dst)
    {
	int offset_bits = 0;
	int buf_delta = (layout.width * layout.height);
	int dst_offs = 0;

	for (int i = 0; i < layout.num_tiles; i++)
	{
	    gfxDecodeChar(layout, src, dst, offset_bits, dst_offs);
	    offset_bits += layout.delta;
	    dst_offs += buf_delta;
	}
    }
};

#endif // BERRN_GFX_H