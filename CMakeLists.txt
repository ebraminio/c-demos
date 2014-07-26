cmake_minimum_required (VERSION 3.0.0)
project (glcourse)

# Deps
## glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE STRING "" FORCE)
set(GLFW_INSTALL OFF CACHE STRING "" FORCE)
add_subdirectory(glfw)
include_directories(glfw/include)

## freetype
add_subdirectory(freetype2)
include_directories(freetype2/include)

## harfbuzz
set(HB_INSTALL OFF CACHE STRING "" FORCE)
include(harfbuzz.cmake)
include_directories(harfbuzz/src)

# Ours
## tut0
add_executable(tut0 tut0.cpp)
target_link_libraries(tut0 glfw ${GLFW_LIBRARIES})

## tut1
add_executable(tut1 tut1.cpp)
target_link_libraries(tut1 harfbuzz freetype)
