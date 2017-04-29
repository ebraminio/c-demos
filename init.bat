@echo off
if not exist freetype2 git clone --depth=1 http://git.savannah.nongnu.org/r/freetype/freetype2.git/
if not exist glfw git clone --depth=1 https://github.com/glfw/glfw
if not exist glew git clone --depth=1 https://github.com/omniavinco/glew-cmake glew
if not exist harfbuzz git clone --depth=1 https://github.com/behdad/harfbuzz
for %%f in (freetype2 glfw glew harfbuzz) do cd %%f & git pull & cd ..

"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.com" .