cmake_minimum_required(VERSION 2.8.0)
#project(harfbuzz)

## HarfBuzz build configurations
option(HB_INSTALL "Generate installation target" ON)
option(HB_HAVE_FREETYPE "Use FreeType" OFF)
option(HB_HAVE_UNISCRIBE "Uniscribe shaper" OFF)
option(HB_HAVE_CORETEXT "Enable CoreText shaper on Mac" ON)
option(HB_BUILTIN_UCDN "Use HarfBuzz provided UCDN" ON)

include_directories(AFTER
  ${PROJECT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/.
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  )

add_definitions(-DHAVE_OT)
add_definitions(-DHAVE_ATEXIT)
add_definitions(-DHB_NO_MT)
add_definitions(-DHB_DISABLE_DEPRECATED)
##

## execute ragel tasks
find_program(RAGEL "ragel")

if (RAGEL)
  message(STATUS "ragel found at: ${RAGEL}")
else (RAGEL)
  message(FATAL_ERROR "ragel not found, get it here -- http://www.complang.org/ragel/")
endif (RAGEL)

function (ragel_preproc src_dir src_sans_rl out_sfx)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src_sans_rl}${out_sfx}
    COMMAND ${RAGEL} -G2 -o ${CMAKE_CURRENT_BINARY_DIR}/${src_sans_rl}${out_sfx} ${CMAKE_CURRENT_SOURCE_DIR}/${src_dir}/${src_sans_rl}.rl -I ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src_dir}/${src_sans_rl}.rl
    )
  add_custom_target(harfbuzz_${src_sans_rl} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${src_sans_rl})
endfunction(ragel_preproc)

ragel_preproc(harfbuzz/src hb-buffer-deserialize-json .hh)
ragel_preproc(harfbuzz/src hb-buffer-deserialize-text .hh)
ragel_preproc(harfbuzz/src hb-ot-shape-complex-indic-machine .hh)
ragel_preproc(harfbuzz/src hb-ot-shape-complex-myanmar-machine .hh)
ragel_preproc(harfbuzz/src hb-ot-shape-complex-sea-machine .hh)
##

## Make hb-version.h
file(READ harfbuzz/configure.ac CONFIGUREAC)
string(REGEX MATCH "\\[(([0-9]+)\\.([0-9]+)\\.([0-9]+))\\]" HB_VERSION_MATCH ${CONFIGUREAC})
set (HB_VERSION ${CMAKE_MATCH_1})
set (HB_VERSION_MAJOR ${CMAKE_MATCH_2})
set (HB_VERSION_MINOR ${CMAKE_MATCH_3})
set (HB_VERSION_MICRO ${CMAKE_MATCH_4})

set(HB_VERSION_H_IN "${PROJECT_SOURCE_DIR}/harfbuzz/src/hb-version.h.in")
set(HB_VERSION_H "${PROJECT_BINARY_DIR}/harfbuzz/hb-version.h")
set_source_files_properties("${HB_VERSION_H}" PROPERTIES GENERATED true)
configure_file("${HB_VERSION_H_IN}" "${HB_VERSION_H}.tmp" @ONLY)
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
  "${HB_VERSION_H}.tmp"
  "${HB_VERSION_H}")
file(REMOVE "${HB_VERSION_H}.tmp")
include_directories("${PROJECT_BINARY_DIR}/harfbuzz/")
##

## Define source and headers of projects
set(project_sources
  ${project_sources}

  ${CMAKE_CURRENT_BINARY_DIR}/hb-buffer-deserialize-json.hh
  ${CMAKE_CURRENT_BINARY_DIR}/hb-buffer-deserialize-text.hh
  ${CMAKE_CURRENT_BINARY_DIR}/hb-ot-shape-complex-indic-machine.hh
  ${CMAKE_CURRENT_BINARY_DIR}/hb-ot-shape-complex-myanmar-machine.hh
  ${CMAKE_CURRENT_BINARY_DIR}/hb-ot-shape-complex-sea-machine.hh
  )

set(project_headers
  ${HB_VERSION_H}

  harfbuzz/src/hb.h
  harfbuzz/src/hb-blob.h
  harfbuzz/src/hb-buffer.h
  harfbuzz/src/hb-common.h
  harfbuzz/src/hb-deprecated.h
  harfbuzz/src/hb-face.h
  harfbuzz/src/hb-font.h
  harfbuzz/src/hb-ot.h
  harfbuzz/src/hb-ot-layout.h
  harfbuzz/src/hb-ot-tag.h
  harfbuzz/src/hb-set.h
  harfbuzz/src/hb-shape.h
  harfbuzz/src/hb-shape-plan.h
  harfbuzz/src/hb-unicode.h
  )

set(project_sources
  ${project_sources}

  harfbuzz/src/hb-blob.cc
  harfbuzz/src/hb-buffer.cc
  harfbuzz/src/hb-buffer-serialize.cc
  harfbuzz/src/hb-common.cc
  harfbuzz/src/hb-face.cc
  harfbuzz/src/hb-font.cc
  harfbuzz/src/hb-ot-tag.cc
  harfbuzz/src/hb-set.cc
  harfbuzz/src/hb-shape.cc
  harfbuzz/src/hb-shape-plan.cc
  harfbuzz/src/hb-shaper.cc
  harfbuzz/src/hb-unicode.cc
  harfbuzz/src/hb-warning.cc

  harfbuzz/src/hb-atomic-private.hh
  harfbuzz/src/hb-buffer-private.hh
  harfbuzz/src/hb-cache-private.hh
  harfbuzz/src/hb-face-private.hh
  harfbuzz/src/hb-font-private.hh
  harfbuzz/src/hb-mutex-private.hh
  harfbuzz/src/hb-object-private.hh
  harfbuzz/src/hb-open-file-private.hh
  harfbuzz/src/hb-open-type-private.hh
  harfbuzz/src/hb-ot-head-table.hh
  harfbuzz/src/hb-ot-hhea-table.hh
  harfbuzz/src/hb-ot-hmtx-table.hh
  harfbuzz/src/hb-ot-maxp-table.hh
  harfbuzz/src/hb-ot-name-table.hh
  harfbuzz/src/hb-private.hh
  harfbuzz/src/hb-set-private.hh
  harfbuzz/src/hb-shape-plan-private.hh
  harfbuzz/src/hb-shaper-impl-private.hh
  harfbuzz/src/hb-shaper-list.hh
  harfbuzz/src/hb-shaper-private.hh
  harfbuzz/src/hb-unicode-private.hh
  harfbuzz/src/hb-utf-private.hh

  # Open Type
  harfbuzz/src/hb-ot-layout.cc
  harfbuzz/src/hb-ot-map.cc
  harfbuzz/src/hb-ot-shape.cc
  harfbuzz/src/hb-ot-shape-complex-arabic.cc
  harfbuzz/src/hb-ot-shape-complex-default.cc
  harfbuzz/src/hb-ot-shape-complex-hangul.cc
  harfbuzz/src/hb-ot-shape-complex-hebrew.cc
  harfbuzz/src/hb-ot-shape-complex-indic.cc
  harfbuzz/src/hb-ot-shape-complex-indic-table.cc
  harfbuzz/src/hb-ot-shape-complex-myanmar.cc
  harfbuzz/src/hb-ot-shape-complex-sea.cc
  harfbuzz/src/hb-ot-shape-complex-thai.cc
  harfbuzz/src/hb-ot-shape-complex-tibetan.cc
  harfbuzz/src/hb-ot-shape-fallback.cc
  harfbuzz/src/hb-ot-shape-normalize.cc

  harfbuzz/src/hb-ot-layout-common-private.hh
  harfbuzz/src/hb-ot-layout-gdef-table.hh
  harfbuzz/src/hb-ot-layout-gpos-table.hh
  harfbuzz/src/hb-ot-layout-gsubgpos-private.hh
  harfbuzz/src/hb-ot-layout-gsub-table.hh
  harfbuzz/src/hb-ot-layout-jstf-table.hh
  harfbuzz/src/hb-ot-layout-private.hh
  harfbuzz/src/hb-ot-map-private.hh
  harfbuzz/src/hb-ot-shape-complex-arabic-fallback.hh
  harfbuzz/src/hb-ot-shape-complex-arabic-table.hh
  harfbuzz/src/hb-ot-shape-complex-indic-private.hh
  harfbuzz/src/hb-ot-shape-complex-private.hh
  harfbuzz/src/hb-ot-shape-fallback-private.hh
  harfbuzz/src/hb-ot-shape-normalize-private.hh
  harfbuzz/src/hb-ot-shape-private.hh

  harfbuzz/src/hb-ot-shape.h
  )

if (HB_HAVE_FREETYPE)
  set(FREETYPE_DIR "$ENV{FREETYPE_DIR}"
    CACHE PATH "root path for freetype lib/ and include/ folders"
    )
  find_path(FREETYPE_INCLUDE_DIR
    ft2build.h freetype2/freetype/freetype.h
    PATHS ${FREETYPE_DIR}/include
    )
  find_library(FREETYPE_LIBRARY
    freetype libfreetype
    PATHS ${FREETYPE_DIR}/lib
    DOC "freetype library"
    )
  if (FREETYPE_INCLUDE_DIR)
    include_directories(AFTER ${FREETYPE_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIR}/freetype2)
  endif ()

  if (FREETYPE_INCLUDE_DIR AND FREETYPE_LIBRARY)
    set (THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} ${FREETYPE_LIBRARY})
    add_definitions(-DHAVE_FREETYPE=1 -DHAVE_FT_FACE_GETCHARVARIANTINDEX=1)
  endif ()

  set(project_headers ${project_headers} harfbuzz/src/hb-ft.h)

  set(project_sources ${project_sources} harfbuzz/src/hb-ft.cc)
endif ()

if (HB_BUILTIN_UCDN)
  include_directories(harfbuzz/src/hb-ucdn)
  add_definitions(-DHAVE_UCDN)
  
  set(project_headers ${project_headers} harfbuzz/src/hb-ucdn/ucdn.h)

  set(project_sources
    ${project_sources}

    harfbuzz/src/hb-ucdn.cc
    harfbuzz/src/hb-ucdn/ucdn.c
    harfbuzz/src/hb-ucdn/unicodedata_db.h)
else ()
  add_definitions(-DHB_NO_UNICODE_FUNCS)
endif ()

if (APPLE AND HB_HAVE_CORETEXT)
  # Apple Advanced Typography
  add_definitions(-DHAVE_CORETEXT)

  set(project_sources
    ${project_sources}
    harfbuzz/src/hb-coretext.cc)

  find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices)
  mark_as_advanced(APPLICATION_SERVICES_FRAMEWORK)
  if (APPLICATION_SERVICES_FRAMEWORK)
    set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} ${APPLICATION_SERVICES_FRAMEWORK})
  endif (APPLICATION_SERVICES_FRAMEWORK)
endif ()

if (WIN32 AND HB_HAVE_UNISCRIBE)
  add_definitions(-DHAVE_UNISCRIBE)

  set(project_headers ${project_headers} harfbuzz/src/hb-uniscribe.h)

  set(project_sources ${project_sources} harfbuzz/src/hb-uniscribe.cc)

  set(THIRD_PARTY_LIBS ${THIRD_PARTY_LIBS} usp10 gdi32 rpcrt4)
endif ()

set(project_sources ${project_sources} ${project_headers})
##

add_library(harfbuzz STATIC ${project_sources})
target_link_libraries(harfbuzz ${THIRD_PARTY_LIBS})

if (HB_INSTALL)
  install(TARGETS harfbuzz DESTINATION lib)
  install(FILES
    ${project_headers}

    DESTINATION
    include/harfbuzz)
endif()
