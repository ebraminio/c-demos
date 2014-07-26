#!/bin/sh
rm -rf build
mkdir build
cd build
cmake -G "Unix Makefiles" ../
cd ..