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

#ifndef LIBBLUEBERRN_API_H
#define LIBBLUEBERRN_API_H

#if defined(_MSC_VER) && !defined(LIBBLUEBERRN_STATIC)
    #ifdef LIBBLUEBERRN_EXPORTS
        #define LIBBLUEBERRN_API __declspec(dllexport)
    #else
        #define LIBBLUEBERRN_API __declspec(dllimport)
    #endif // LIBBLUEBERRN_EXPORTS
#else
    #define LIBBLUEBERRN_API
#endif // _MSC_VER

#endif // LIBMBGB_API_H