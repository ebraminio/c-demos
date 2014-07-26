@echo off
rmdir /s /q build 2>NUL
mkdir build
cd build
cmake -G "Visual Studio 12" ../
cd..