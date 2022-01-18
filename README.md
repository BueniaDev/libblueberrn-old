<img src="https://github.com/BueniaDev/libblueberrn/blob/main/res/logo.bmp" alt="drawing" width="300"/>

[![CircleCI](https://circleci.com/gh/BueniaDev/libblueberrn.svg?style=svg)](https://circleci.com/gh/BueniaDev/libblueberrn)

Arcade machine emulation library, sorta

The goal here is to help preserve the arcade machines of our past. But, also, you know, to have a fun challenge... :)

# Current Status

As of this time, libblueberrn is already capable of playing a few commercial arcade ROMs, and more ROMs are beginning to boot. Please see the compatibility database for more details.

[Link to compatibility database](https://github.com/BueniaDev/libblueberrn-compatibility)

# Building Instructions

The libblueberrn library does not have any external dependencies and can be compiled with MinGW or Clang on Windows, GCC or Clang on Linux, and (presumably) AppleClang on Mac OS. The examples contained in this repo, however, do have additional dependencies that need to be installed. All dependencies should be findable by CMake.

## Linux:

Step 1: Install dependencies:

Core dependencies:

Compiler: GCC or Clang. You only need one of those two:

GCC 9.3.0+ (earlier versions not tested):

Debian: `sudo apt-get install build-essential`

Arch (not tested): `sudo pacman -S base-devel`

Fedora (not tested): `sudo dnf install gcc-c++`

OpenSUSE (not tested): `sudo zypper in gcc-c++`

Clang:

Debian: `sudo apt-get install clang clang-format libc++-dev` (in some distros, clang-5.0)

Arch (not tested): `pacman -S clang`, `libc++` is in the AUR. Use pacaur or yaourt to install it.

Fedora (not tested): `dnf install clang libcxx-devel`

OpenSUSE (not tested): `zypper in clang`

Git (if not installed already) and CMake 3.13+:

Debian: `sudo apt-get install git cmake`

Arch (not tested): `sudo pacman -S git`

Fedora (not tested): `sudo dnf install git cmake`

OpenSUSE (not tested): `sudo zypper in git cmake extra-cmake-modules`

For the SDL2 frontend:

Debian: `sudo apt-get install libsdl2-dev`

Arch (not tested): `sudo pacman -S sdl2`

Fedora (not tested): `sudo dnf install SDL2-devel`

OpenSUSE: (not tested): `sudo zypper in libSDL2-devel`

Step 2: Clone the repository:

```
git clone --recursive https://github.com/BueniaDev/libblueberrn.git
cd libblueberrn
```

Step 3: Compile:

```
mkdir build && cd build
cmake .. -G "Unix Makefiles" -DBUILD_EXAMPLE="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>"
make -j$(nproc --all)
```

## Mac OS (not tested):

You will need [homebrew](https://brew.sh), a recent version of Xcode and the Xcode command-line tools to build libblueberrn.
Please note that due to personal financial constraints, libblueberrn has not been thoroughly tested on Mac OS as of yet.

Step 1: Install dependencies:

```
brew install git cmake pkg-config
```

For the SDL2 frontend:

```
brew install sdl2
```

Step 2: Clone the repository:

```
git clone --recursive https://github.com/BueniaDev/libblueberrn.git
cd libblueberrn
```

Step 3: Compile:

```
mkdir build && cd build
cmake .. -G "Unix Makefiles" -DBUILD_EXAMPLE="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>""
make -j$(sysctl -n hw.ncpu)
```

## Windows:

You will need [MSYS2](https://msys2.github.io) in order to install libblueberrn.
Make sure to run `pacman -Syu` as needed.

Step 1: Install dependencies:

```
pacman -S base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake git
```

For the SDL2 frontend:

```
pacman -S mingw-w64-x86_64-SDL2
```

Step 2: Clone the repository:

```
git clone --recursive https://github.com/BueniaDev/libblueberrn.git
cd libblueberrn
```

Step 3: Compile:

```
mkdir build && cd build
cmake .. -G "MSYS Makefiles" -DBUILD_EXAMPLE="<ON/OFF>" -DCMAKE_BUILD_TYPE="<Debug/Release>""
(mingw32-)make -j$(nproc --all)
../msys-dist.sh
```


# Plans

## Near-term

Improve Midway Space Invaders hardware support

Improve Namco Galaxian hardware support

Improve Namco Pacman hardware support

Improve Sega System 1 hardware support

Improve Namco Galaga hardware support

Taito 68k hardware support

## Medium-term

Sega System 16 hardware support

Capcom CPS-1 support

Capcom Mitchell hardware support

## Long-term

Capcom CPS-2 support

Sega Super Scaler hardware support

Sega System 32 and Model 1 hardware support

Taito B, X and Z System support


# License

<img src="https://www.gnu.org/graphics/gplv3-127x51.png" alt="drawing" width="150"/>

libblueberrn is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

# Copyright

(C) 2022 BueniaDev. This project is not affiliated in any way with Taito, Namco, Midway, or any of the manufacturers of the arcade machines supported in this project. All arcade games and their logos/brandings featured herein are the registered trademarks of their respective owners.

All copyrighted material in this repository belongs to their respective owners, and is used under the terms of the "fair use" clause of U.S. and international copyright law. For more information, see the "res/fairuse.txt" file in this repository.

For information regarding libblueberrn's stance on copyright infringement, see the LEGAL.md file in this repository.
