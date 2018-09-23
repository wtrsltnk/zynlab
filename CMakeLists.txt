cmake_minimum_required(VERSION 2.8)

project(zynlab)

option(BUILD_VST "enable building all projects using VST" OFF)

# Add version information
set(VERSION "2.4.4")
add_definitions(-DVERSION="${VERSION}")

find_package(MXML REQUIRED)
find_package(FLTK)
find_package(OpenGL) #for FLTK

if(FLTK_FOUND)
    set(FltkGui TRUE)
    add_definitions(-DENABLE_FLTKGUI)
else()
    message(STATUS "GUI module defaulting to off")
endif()

add_subdirectory("zyn.common")
add_subdirectory("zyn.dsp")
add_subdirectory("zyn.fx")
add_subdirectory("zyn.mixer")
add_subdirectory("zyn.net")
add_subdirectory("zyn.nio")
add_subdirectory("zyn.seq")
add_subdirectory("zyn.synth")
add_subdirectory("app")
add_subdirectory("app.threedee")
add_subdirectory("app.imgui")

if(WIN32 AND BUILD_VST)
	add_subdirectory("zyn.vst")
	add_subdirectory("vst.instrument")
	add_subdirectory("vst.effect")
endif()
