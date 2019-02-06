cmake_minimum_required(VERSION 3.8)

project(zynlab)

option(BUILD_VST "enable building all projects using VST" OFF)
option(BUILD_TESTS "enable building all test projects" OFF)
option(BUILD_APP_DRUMMER "enable building app.drummer project" OFF)
option(BUILD_APP_IMGUI "enable building app.imgui project" OFF)
option(BUILD_APP_TINY "enable building app.tiny project" OFF)

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
add_subdirectory("zyn.serialization")
add_subdirectory("app")
add_subdirectory("app.threedee")

if(BUILD_APP_TINY)
    add_subdirectory("app.tiny")
endif()

if (BUILD_APP_IMGUI)
    add_subdirectory("app.imgui")
endif()

if(BUILD_APP_DRUMMER)
    add_subdirectory("app.drummer")
endif()

if(BUILD_TESTS)
    add_subdirectory("tests")
endif()

if(WIN32 AND BUILD_VST)
    add_subdirectory("zyn.vst")
    add_subdirectory("zyn.vstadd")
    add_subdirectory("zyn.vstsub")
    add_subdirectory("zyn.vstpad")
endif()
