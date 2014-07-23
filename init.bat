@echo off
rmdir /s /q build > NUL
mkdir build
cd build
cmake -G "Visual Studio 12" ../
cd..