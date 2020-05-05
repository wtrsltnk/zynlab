cmake_minimum_required(VERSION 3.12)

project(zynlab)

option(BUILD_VST "enable building all projects using VST" OFF)
option(BUILD_TESTS "enable building all test projects" OFF)
option(BUILD_APP_ZYNADSUBFX "enable building app project" ON)
option(BUILD_APP_THREEDEE "enable building app.threedee project" ON)

# Add version information
set(VERSION "2.4.4")
add_definitions(-DVERSION="${VERSION}")

find_package(FLTK)
find_package(OpenGL) #for FLTK

if(FLTK_FOUND)
    set(FltkGui TRUE)
    add_definitions(-DENABLE_FLTKGUI)
else()
    message(STATUS "GUI module defaulting to off")
endif()

add_subdirectory("lib.imgui")
add_subdirectory("lib.mxml")

add_subdirectory("zyn.common")
add_subdirectory("zyn.dsp")
add_subdirectory("zyn.fx")
add_subdirectory("zyn.mixer")
add_subdirectory("zyn.net")
add_subdirectory("zyn.nio")
add_subdirectory("zyn.seq")
add_subdirectory("zyn.synth")
add_subdirectory("zyn.serialization")
add_subdirectory("zyn.pinvoke")

if(BUILD_APP_ZYNADSUBFX)
    add_subdirectory("app")
endif()

if(BUILD_APP_THREEDEE)
    add_subdirectory("app.threedee")
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
