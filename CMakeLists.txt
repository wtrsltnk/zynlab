cmake_minimum_required(VERSION 3.12)

project(zynlab VERSION "2.4.4")

option(BUILD_VST "enable building all projects using VST" OFF)
option(BUILD_TESTS "enable building all test projects" OFF)
option(BUILD_APP_ZYNADSUBFX "enable building app project" ON)
option(BUILD_APP_THREEDEE "enable building app.threedee project" ON)
option(BUILD_APP_MIXER "enable building app.mixer project" ON)
option(BUILD_APP_TRACKER "enable building app.tracker project" ON)
option(BUILD_APP_SYNTH "enable building app.synth project" ON)
option(BUILD_PINVOKE "enable building pinvoke project" OFF)

include(cmake/CPM.cmake)
include(cmake/Dependencies.cmake)

add_definitions(-DVERSION="${CMAKE_PROJECT_VERSION}")
add_definitions(-DENABLE_FLTKGUI)

find_package(OpenGL) #for FLTK


add_library(_docs INTERFACE README.md)

add_subdirectory("lib.mxml")
add_subdirectory("lib.sqlite3")

add_subdirectory("zyn.common")
add_subdirectory("zyn.dsp")
add_subdirectory("zyn.fx")
add_subdirectory("zyn.mixer")
add_subdirectory("zyn.net")
add_subdirectory("zyn.nio")
add_subdirectory("zyn.seq")
add_subdirectory("zyn.synth")
add_subdirectory("zyn.serialization")
add_subdirectory("zyn.ui")

if(BUILD_APP_ZYNADSUBFX)
    add_subdirectory("app")
endif()

if (BUILD_APP_SYNTH)
    add_subdirectory("app.synth")
endif(BUILD_APP_SYNTH)

if(BUILD_APP_THREEDEE)
    add_subdirectory("lib.imgui")
    add_subdirectory("app.threedee")
endif()

if(BUILD_APP_MIXER)
    add_subdirectory("app.mixer")
endif()

if(BUILD_APP_TRACKER)
    add_subdirectory("app.tracker")
endif()

if(BUILD_TESTS)
    add_subdirectory("tests")
endif()

if(BUILD_PINVOKE)
    add_subdirectory("zyn.pinvoke")
endif()

if(WIN32 AND BUILD_VST)
    add_subdirectory("zyn.vst")
    add_subdirectory("zyn.vstadd")
    add_subdirectory("zyn.vstsub")
    add_subdirectory("zyn.vstpad")
endif()
