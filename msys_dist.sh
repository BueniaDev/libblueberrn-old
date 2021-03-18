#!/bin/bash

if [[ ! -f "libblueberrn.a" ]]; then
	echo "Run this script from the directory where you built libblueberrn."
	exit 1
fi

mkdir -p dist

if [ -d "example/sdl2" ]; then
	for lib in $(ldd example/sdl2/example.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp example/sdl2/example.exe dist
fi

mkdir -p dist/roms
echo "Place your ROMs (in file or zip form) HERE!" > dist/roms/PLACE-ROMS-HERE.txt