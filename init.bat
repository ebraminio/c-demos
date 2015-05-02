@echo off
if not exist freetype2 git clone --depth=1 http://git.sv.nongnu.org/r/freetype/freetype2.git
if not exist glfw git clone --depth=1 https://github.com/glfw/glfw
if not exist glew git clone --depth=1 https://github.com/omniavinco/glew-cmake glew
if not exist harfbuzz git clone --depth=1 https://github.com/behdad/harfbuzz
for %%f in (freetype2 glfw glew harfbuzz) do git -c %%f pull

rmdir /s /q build 2>NUL
mkdir build
cd build
cmake -G "Visual Studio 14" ../
cd..