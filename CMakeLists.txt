cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

project(zyn)

set(WIN_CPP_PREFIX_PATH $ENV{WIN_CPP_PREFIX_PATH})
if (NOT WIN_CPP_PREFIX_PATH)
    message("Cannot find WIN_CPP_PREFIX_PATH")
endif()
if (NOT EXISTS "${WIN_CPP_PREFIX_PATH}")
    message("${WIN_CPP_PREFIX_PATH} does not exist")
else()
    set(CMAKE_PREFIX_PATH "${WIN_CPP_PREFIX_PATH}")
    set(CMAKE_MODULE_PATH "${WIN_CPP_PREFIX_PATH}/cmake-modules")
    set(ZLIB_ROOT ${WIN_CPP_PREFIX_PATH})
endif()

# Add version information
set(VERSION "2.4.4")
add_definitions(-DVERSION="${VERSION}")

find_package(ZLIB REQUIRED)
find_package(MXML REQUIRED)
find_package(FLTK)
find_package(OpenGL) #for FLTK

if(FLTK_FOUND)
    set(FltkGui TRUE)
else()
    add_definitions(-DDISABLE_GUI)
    message(STATUS "GUI module defaulting to off")
endif()

include_directories(
    ${MXML_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    )

add_subdirectory("zyn.common")
add_subdirectory("zyn.dsp")
add_subdirectory("zyn.fx")
add_subdirectory("zyn.mixer")
add_subdirectory("zyn.nio")
add_subdirectory("zyn.synth")
add_subdirectory("app")
add_subdirectory("app.threedee")

if(WIN32)
	add_subdirectory("zyn.vst")
	add_subdirectory("vst.instrument")
	add_subdirectory("vst.effect")
endif()
