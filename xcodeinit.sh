#!/bin/sh
set -o errexit
set -o pipefail
set -o nounset
# set -o xtrace

[ -d freetype2 ] || git clone --depth=1 http://git.savannah.nongnu.org/r/freetype/freetype2.git/
[ -d glfw ] || git clone --depth=1 https://github.com/glfw/glfw
[ -d glew ] || git clone --depth=1 https://github.com/omniavinco/glew-cmake glew
[ -d harfbuzz ] || git clone --depth=1 https://github.com/behdad/harfbuzz
for f in freetype2 glfw glew harfbuzz; do cd $f; git pull; cd ..; done

rm -rf build
mkdir build
cd build
cmake -G Xcode ../
open glcourse.xcodeproj
cd ..
