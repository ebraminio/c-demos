cmake_minimum_required (VERSION 2.8.12)
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
set(HB_HAVE_FREETYPE ON CACHE STRING "" FORCE)
include(harfbuzz.cmake)
include_directories(harfbuzz/src)

## glew
add_definitions(-DGLEW_STATIC)
option(glew-cmake_BUILD_SHARED "" OFF)
add_subdirectory(glew)
include_directories(glew/include glew/src)

# Ours
## tut0
add_executable(tut0 tut0.cc)
target_link_libraries(tut0 glfw ${GLFW_LIBRARIES})

## tut1
add_executable(tut1 tut1.cc)
target_link_libraries(tut1 harfbuzz freetype)

## tut2
add_executable(tut2 tut2.cc)
target_link_libraries(tut2 glfw ${GLFW_LIBRARIES} libglew_static)

## tut3
add_executable(tut3 tut3.cc)
target_link_libraries(tut3 glfw ${GLFW_LIBRARIES} libglew_static)

## tut4
add_executable(tut4 tut4.cc)
target_link_libraries(tut4 harfbuzz freetype)

## hbftsvg
add_executable(hbftsvg hbftsvg.cc)
target_link_libraries(hbftsvg harfbuzz freetype)

## hello-harfbuzz
add_definitions(-DPNG_OUTPUT)
add_executable(hello-harfbuzz hello-harfbuzz.cc)
target_link_libraries(hello-harfbuzz harfbuzz freetype)

## hello-harfbuzz-gl
add_executable(hello-harfbuzz-gl hello-harfbuzz-gl.cc)
target_link_libraries(hello-harfbuzz-gl harfbuzz freetype glfw ${GLFW_LIBRARIES} libglew_static)
