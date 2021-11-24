#!/bin/bash

if [[ ! -f "libblueberrn.a" ]]; then
	echo "Run this script from the directory where you built libblueberrn."
	exit 1
fi

mkdir -p dist

if [ -d "blueberrn-SDL" ]; then
	for lib in $(ldd blueberrn-SDL/blueberrn-SDL.exe | grep mingw | sed "s/.*=> //" | sed "s/(.*)//"); do
		cp "${lib}" dist
	done
	cp blueberrn-SDL/blueberrn-SDL.exe dist
fi

mkdir -p dist/fonts
cp -r ../blueberrn-SDL/fonts dist
mkdir -p dist/roms
mkdir -p dist/samples
echo "Place your ROMs (in file or zip form) HERE!" > dist/roms/PLACE-ROMS-HERE.txt
echo "Place your samples (in file form) HERE!" > dist/samples/PLACE-SAMPLES-HERE.txt