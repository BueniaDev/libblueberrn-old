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

#ifndef BERRN_GFX_H
#define BERRN_GFX_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <chrono>
#include <utils.h>
#include <libblueberrn_api.h>
using namespace std;

namespace berrn
{
    #define berrn_swapxy (1 << 2)
    #define berrn_flipy (1 << 1)
    #define berrn_flipx (1 << 0)

    #define berrn_rot_mask  0x7
    #define berrn_rot_0     0
    #define berrn_rot_90    (berrn_swapxy | berrn_flipy)
    #define berrn_rot_180   (berrn_flipx | berrn_flipy)
    #define berrn_rot_270   (berrn_swapxy | berrn_flipx)

    struct berrnRGBA
    {
	union
	{
	    struct
	    {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
	    };

	    uint8_t raw[4];
	    uint32_t val;
	};

	berrnRGBA() : red(0), green(0), blue(0), alpha(255)
	{

	}

	berrnRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : red(r), green(g), blue(b), alpha(a)
	{

	}

	berrnRGBA(const berrnRGBA &c) : val(c.val)
	{

	}

	uint8_t& at(int index)
	{
	    if ((index < 0) || (index >= 4))
	    {
		throw out_of_range("Color index out of bounds");
	    }

	    return raw[index];
	}

	uint8_t& operator[](int index)
	{
	    return raw[index];
	}

	berrnRGBA &operator =(const berrnRGBA &c)
	{
	    if (this != &c)
	    {
		val = c.val;
	    }

	    return *this;
	}
    };
	
    inline berrnRGBA red()
    {
	return berrnRGBA(255, 0, 0);
    }
	
    inline berrnRGBA green()
    {
	return berrnRGBA(0, 255, 0);
    }
	
    inline berrnRGBA blue()
    {
	return berrnRGBA(0, 0, 255);
    }

    inline berrnRGBA cyan()
    {
	return berrnRGBA(0, 255, 255);
    }

    inline berrnRGBA magenta()
    {
	return berrnRGBA(255, 0, 255);
    }

    inline berrnRGBA yellow()
    {
	return berrnRGBA(255, 255, 0);
    }

    inline berrnRGBA black()
    {
	return berrnRGBA(0, 0, 0);
    }

    inline berrnRGBA white()
    {
	return berrnRGBA(255, 255, 255);
    }

    inline berrnRGBA fromLevel(int level, int bpp)
    {
	int level_ratio = ((1 << bpp) - 1);
	uint8_t color = ((level * 255) / level_ratio);
	return berrnRGBA(color, color, color);
    }

    inline berrnRGBA fromColor(uint8_t color)
    {
	return berrnRGBA(color, color, color);
    }

    inline berrnRGBA fromRGB(uint8_t red, uint8_t green, uint8_t blue)
    {
	return berrnRGBA(red, green, blue);
    }

    inline berrnRGBA fromRGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    {
	return berrnRGBA(red, green, blue, alpha);
    }

    enum BerrnPaletteFormat
    {
	R8G8B8,
	R8G8B8A8,
	XBGR_555,
	XBGR_444
    };

    class BerrnPalette
    {
	public:
	    BerrnPalette(BerrnPaletteFormat format, size_t num_entries, size_t num_bytes, bool is_big_endian = false) : format_(format), num_entries_(num_entries), num_bytes_(num_bytes), is_big_endian_(is_big_endian)
	    {
		palette_ram.resize(numBytes(), 0);
		colors.resize(numEntries(), black());
	    }

	    virtual ~BerrnPalette()
	    {
		colors.clear();
		palette_ram.clear();
	    }

	    BerrnPaletteFormat format() const
	    {
		return format_;
	    }

	    size_t numEntries() const
	    {
		return num_entries_;
	    }

	    size_t numBytes() const
	    {
		return (numEntries() * numBytesPerEntry());
	    }

	    bool isBigEndian() const
	    {
		return is_big_endian_;
	    }

	    uint8_t read8(size_t offs)
	    {
		if (!inRange(offs, 0, numBytes()))
		{
		    return 0;
		}

		return palette_ram.at(offs);
	    }

	    void write8(size_t offs, uint8_t data)
	    {
		if (!inRange(offs, 0, numBytes()))
		{
		    return;
		}

		palette_ram.at(offs) = data;
		updateColor(offs);
	    }

	    uint16_t read16(bool upper, bool lower, uint32_t addr)
	    {
		uint16_t data = 0;

		if (upper)
		{
		    data |= (read8(addr) << 8);
		}

		if (lower)
		{
		    data |= read8(addr + 1);
		}

		return data;
	    }

	    void write16(bool upper, bool lower, uint32_t addr, uint16_t data)
	    {
		if (upper)
		{
		    write8(addr, (data >> 8));
		}

		if (lower)
		{
		    write8((addr + 1), (data & 0xFF));
		}
	    }

	    size_t numBytesPerEntry() const
	    {
		return num_bytes_;
	    }

	    berrnRGBA getColor(size_t offs)
	    {
		if (!inRange(offs, 0, numEntries()))
		{
		    stringstream ss;
		    ss << "Invalid entry offset of " << dec << offs;
		    throw out_of_range(ss.str());
		}

		return colors.at(offs);
	    }

	    void clear()
	    {
		fill(palette_ram.begin(), palette_ram.end(), 0);
		fill(colors.begin(), colors.end(), black());
	    }

	protected:
	    uint16_t read16BE(size_t offs)
	    {
		uint8_t high = read8(offs);
		uint8_t low = read8(offs + 1);

		return ((high << 8) | low);
	    }

	    uint16_t read16LE(size_t offs)
	    {
		uint8_t high = read8(offs + 1);
		uint8_t low = read8(offs);

		return ((high << 8) | low);
	    }

	    virtual berrnRGBA convertColor(size_t offs) = 0;

	private:
	    BerrnPaletteFormat format_;
	    size_t num_entries_;
	    size_t num_bytes_;
	    bool is_big_endian_;

	    void updateColor(size_t offs)
	    {
		if (numBytesPerEntry() == 0)
		{
		    throw out_of_range("Invalid number of bytes per entry");
		}

		size_t entry_offs = (offs / numBytesPerEntry());
		colors.at(entry_offs) = convertColor(entry_offs);
	    }

	    vector<uint8_t> palette_ram;
	    vector<berrnRGBA> colors;
    };

    class BerrnPaletteR8G8B8 : public BerrnPalette
    {
	public:
	    BerrnPaletteR8G8B8(size_t num_entries) : BerrnPalette(R8G8B8, num_entries, 3)
	    {

	    }

	    ~BerrnPaletteR8G8B8()
	    {

	    }

	    berrnRGBA convertColor(size_t offs)
	    {
		if (!inRange(offs, 0, numEntries()))
		{
		    return black();
		}

		size_t color_offs = (offs * 3);

		int red = read8(color_offs);
		int green = read8(color_offs + 1);
		int blue = read8(color_offs + 2);

		return fromRGB(red, green, blue);
	    }
    };

    class BerrnPaletteR8G8B8A8 : public BerrnPalette
    {
	public:
	    BerrnPaletteR8G8B8A8(size_t num_entries) : BerrnPalette(R8G8B8A8, num_entries, 4)
	    {

	    }

	    ~BerrnPaletteR8G8B8A8()
	    {

	    }

	    berrnRGBA convertColor(size_t offs)
	    {
		if (!inRange(offs, 0, numEntries()))
		{
		    return black();
		}

		size_t color_offs = (offs * 4);

		int red = read8(color_offs);
		int green = read8(color_offs + 1);
		int blue = read8(color_offs + 2);
		int alpha = read8(color_offs + 3);

		return fromRGBA(red, green, blue, alpha);
	    }
    };

    class BerrnPaletteXBGR555 : public BerrnPalette
    {
	public:
	    BerrnPaletteXBGR555(size_t num_entries, bool is_big_endian = false) : BerrnPalette(XBGR_555, num_entries, 2, is_big_endian)
	    {

	    }

	    berrnRGBA convertColor(size_t offs)
	    {
		if (!inRange(offs, 0, numEntries()))
		{
		    return black();
		}

		size_t pal_offs = (offs * 2);
		uint16_t color_val = (isBigEndian() ? read16BE(pal_offs) : read16LE(pal_offs));

		int red_val = (color_val & 0x1F);
		int green_val = ((color_val >> 5) & 0x1F);
		int blue_val = ((color_val >> 10) & 0x1F);

		int red = ((red_val << 3) | (red_val >> 2));
		int green = ((green_val << 3) | (green_val >> 2));
		int blue = ((blue_val << 3) | (blue_val >> 2));

		return fromRGB(red, green, blue);
	    }
    };

    class BerrnPaletteXBGR444 : public BerrnPalette
    {
	public:
	    BerrnPaletteXBGR444(size_t num_entries, bool is_big_endian = false) : BerrnPalette(XBGR_444, num_entries, 2, is_big_endian)
	    {

	    }

	    berrnRGBA convertColor(size_t offs)
	    {
		if (!inRange(offs, 0, numEntries()))
		{
		    return black();
		}

		size_t pal_offs = (offs * 2);
		uint16_t color_val = (isBigEndian() ? read16BE(pal_offs) : read16LE(pal_offs));

		int red_val = (color_val & 0xF);
		int green_val = ((color_val >> 4) & 0xF);
		int blue_val = ((color_val >> 8) & 0xF);

		int red = ((red_val << 4) | red_val);
		int green = ((green_val << 4) | green_val);
		int blue = ((blue_val << 4) | blue_val);

		return fromRGB(red, green, blue);
	    }
    };

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

	    ~BerrnBitmapRGB()
	    {
		framebuffer.clear();
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
		    framebuffer.at(x + (y * width())) = color;
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

    inline BerrnBitmap* rotateBitmapRGB(BerrnBitmapRGB *bmp, int rot_flags)
    {
	if (bmp == NULL)
	{
	    return NULL;
	}

	int bmp_width = bmp->width();
	int bmp_height = bmp->height();

	int width = bmp_width;
	int height = bmp_height;

	bool is_swapxy = (rot_flags & berrn_swapxy);

	if (rot_flags & berrn_swapxy)
	{
	    swap(width, height);
	}

	bool is_flipx = (rot_flags & berrn_flipx);
	bool is_flipy = (rot_flags & berrn_flipy);

	BerrnBitmapRGB *bitmap = new BerrnBitmapRGB(width, height);
	bitmap->clear();

	int bmp_w = (bmp_width - 1);
	int bmp_h = (bmp_height - 1);

	for (int x = 0; x < bmp_width; x++)
	{
	    int xpos = is_flipx ? (bmp_w - x) : x;
	    for (int y = 0; y < bmp_height; y++)
	    {
		int ypos = is_flipy ? (bmp_h - y) : y;

		berrnRGBA pixel = bmp->pixel(x, y);

		if (is_swapxy)
		{
		    bitmap->setPixel(ypos, xpos, pixel);
		}
		else
		{
		    bitmap->setPixel(xpos, ypos, pixel);
		}
	    }
	}

	return bitmap;
    }

    #define gfx_step2(start, step) start, (start + step)
    #define gfx_step4(start, step) gfx_step2(start, step), gfx_step2((start + (2 * step)), step)
    #define gfx_step8(start, step) gfx_step4(start, step), gfx_step4((start + (4 * step)), step)
    #define gfx_step16(start, step) gfx_step8(start, step), gfx_step8((start + (8 * step)), step)
    #define gfx_step32(start, step) gfx_step16(start, step), gfx_step16((start + (16 * step)), step)

    struct BerrnGfxLayout
    {
	int width; // Width of a tile (in pixels)
	int height; // Height of a tile (in pixels)
	int num_tiles; // Number of total tiles
	int num_planes; // Number of bit planes
	array<int, 4> plane_offs; // Offset of bit planes (in bits)
	array<int, 32> x_offs; // Offset of pixel x-coordinate in bits relative to one bitplane (in bits)
	array<int, 32> y_offs; // Offset of pixel y-coordinate in bits relative to one bitplane (in bits)
	int delta; // Length of an individual tile (in bits)
    };

    class BerrnGfx
    {
	public:
	    BerrnGfx()
	    {

	    }

	    ~BerrnGfx()
	    {
		gfx_data.clear();
	    }

	    void setLayout(BerrnGfxLayout &layout, vector<uint8_t> src)
	    {
		gfx_layout = layout;
		gfx_rom = src;

		line_mod = gfx_layout.width;
		char_mod = (line_mod * gfx_layout.height);
		gfx_data.resize((gfx_layout.num_tiles * char_mod), 0);
	    }

	    void decode()
	    {
		for (int i = 0; i < gfx_layout.num_tiles; i++)
		{
		    decode_tile(i);
		}
	    }

	    vector<uint8_t> get_gfx_data()
	    {
		return gfx_data;
	    }

	private:
	    BerrnGfxLayout gfx_layout;
	    vector<uint8_t> gfx_rom;
	    vector<uint8_t> gfx_data;

	    uint32_t line_mod = 0;
	    uint32_t char_mod = 0;

	    void decode_tile(int tile_num)
	    {
		uint32_t base_offs = (tile_num * char_mod);

		int plane_bit = (gfx_layout.num_planes - 1);
		for (int plane = 0; plane < gfx_layout.num_planes; plane++, plane_bit--)
		{
		    int plane_offs = (tile_num * gfx_layout.delta + gfx_layout.plane_offs[plane]);

		    for (int ypos = 0; ypos < gfx_layout.height; ypos++)
		    {
			int yoffset = (plane_offs + gfx_layout.y_offs[ypos]);
			uint32_t decode_offs = (base_offs + ypos * line_mod);

			for (int xpos = 0; xpos < gfx_layout.width; xpos++)
			{
			    uint32_t final_offs = (yoffset + gfx_layout.x_offs[xpos]);

			    uint32_t byte_num = (final_offs / 8);
			    int bit_num = (7 - (final_offs % 8));

			    if (testbit(gfx_rom[byte_num], bit_num))
			    {
				uint32_t bit_offs = (decode_offs + xpos);
				gfx_data[bit_offs] = setbit(gfx_data[bit_offs], plane_bit);
			    }
			}
		    }
		}
	    }
    };

    inline void gfxDecodeSet(BerrnGfxLayout &layout, vector<uint8_t> src, vector<uint8_t> &dst)
    {
	BerrnGfx gfx_tiles;
	gfx_tiles.setLayout(layout, src);
	gfx_tiles.decode();
	dst = gfx_tiles.get_gfx_data();
    }
};

#endif // BERRN_GFX_H