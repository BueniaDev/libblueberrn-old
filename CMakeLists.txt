cmake_minimum_required(VERSION 3.0.2)
project(libblueberrn)

option(BUILD_8080 "Enables the Intel 8080 test program." OFF)
option(BUILD_EXAMPLE "Enables the WIP SDL2 frontend (requires SDL2)." OFF)

if (BUILD_8080 STREQUAL "ON")
	add_subdirectory(example/8080test)
endif()

if (BUILD_EXAMPLE STREQUAL "ON")
	add_subdirectory(example/sdl2)
endif()

if (WIN32)
	message(STATUS "Operating system is Windows.")
elseif(UNIX AND NOT APPLE)
	message(STATUS "Operating system is Linux.")
elseif(APPLE)
	message(STATUS "Operating system is Mac.")
else()
	message(SEND_ERROR "Operating system not supported.")
	return()
endif()

set(LIBBLUEBERRN_API_HEADERS
	libblueberrn/include/libblueberrn/berrn/drivers/midway/invaders.h
	libblueberrn/include/libblueberrn/berrn/drivers/namco/galaxian/galaxian.h
	libblueberrn/include/libblueberrn/berrn/drivers/example/example.h
	libblueberrn/include/libblueberrn/berrn/berrn.h
	libblueberrn/include/libblueberrn/berrn/drivers/drivers.h
	libblueberrn/include/libblueberrn/machine/mb14241.h
	libblueberrn/src/core/cpu/z80/z80.h
	libblueberrn/src/core/cpu/8080/8080.h
	libblueberrn/src/core/cpu/m68k/m68k.h
	libblueberrn/src/core/cpu/m68k/utils.h
	libblueberrn/include/libblueberrn/libblueberrn.h)

set(LIBBLUEBERRN
	libblueberrn/src/core/cpu/m68k/m68k.cpp
	libblueberrn/src/core/cpu/m68k/m68kdebug.cpp
	libblueberrn/src/core/cpu/m68k/m68kops.cpp
	libblueberrn/src/core/cpu/m68k/utils.cpp
	libblueberrn/src/core/cpu/8080/8080.cpp
	libblueberrn/src/core/cpu/8080/8080-ops.cpp
	libblueberrn/src/core/cpu/z80/z80.cpp
	libblueberrn/src/core/cpu/z80/z80-ops.cpp
	libblueberrn/src/core/berrn/berrn.cpp
	libblueberrn/src/core/berrn/driverfuncs.cpp
	libblueberrn/src/core/berrn/drivers/midway/invaders.cpp
	libblueberrn/src/core/berrn/drivers/namco/galaxian/galaxian.cpp
	libblueberrn/src/core/berrn/drivers/example/example.cpp
	libblueberrn/src/core/libblueberrn.cpp)

if (WIN32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security -DLIBMBSEGA_EXPORTS")
		message(STATUS "Compiler is MinGW.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (UNIX AND NOT APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security")
		message(STATUS "Compiler is GCC.")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security")
		message(STATUS "Compiler is Clang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
elseif (APPLE)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -Wno-format-security")
		message(STATUS "Compiler is AppleClang.")
	else()
		message(SEND_ERROR "Compiler not supported.")
		return()
	endif()
endif()

if (WIN32)
	add_library(blueberrn STATIC ${LIBBLUEBERRN} ${LIBBLUEBERRN_API_HEADERS})
else()
	add_library(blueberrn ${LIBBLUEBERRN} ${LIBBLUEBERRN_API_HEADERS})
endif()
add_library(libblueberrn ALIAS blueberrn)
