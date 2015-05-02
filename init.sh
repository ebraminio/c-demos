#!/bin/sh
[ -d freetype2 ] || git clone --depth=1 http://git.sv.nongnu.org/r/freetype/freetype2.git
[ -d glfw ] || git clone --depth=1 https://github.com/glfw/glfw
[ -d glew ] || git clone --depth=1 https://github.com/omniavinco/glew-cmake glew
[ -d harfbuzz ] || git clone --depth=1 https://github.com/behdad/harfbuzz
for f in freetype2 glfw glew harfbuzz; do git -c $f pull; done

rm -rf build
mkdir build
cd build
cmake -G "Unix Makefiles" ../
cd ..