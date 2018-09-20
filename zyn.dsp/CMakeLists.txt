cmake_minimum_required(VERSION 3.8)

project(zyn.dsp)

add_library(zyn.dsp STATIC
    AnalogFilter.cpp
    AnalogFilter.h
    Filter.cpp
    Filter.h
    FilterParams.cpp
    FilterParams.h
    FormantFilter.cpp
    FormantFilter.h
    SVFilter.cpp
    SVFilter.h
    Unison.cpp
    Unison.h
    )

target_include_directories(zyn.dsp
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_compile_features(zyn.dsp
    PRIVATE cxx_auto_type
    PRIVATE cxx_nullptr
    )

target_link_libraries(zyn.dsp
    zyn.common
    )

install(
    TARGETS
        zyn.dsp
    EXPORT
        zyn.dsp
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.dsp_SOURCE_DIR}/AnalogFilter.h
        ${zyn.dsp_SOURCE_DIR}/Filter.h
        ${zyn.dsp_SOURCE_DIR}/FilterParams.h
        ${zyn.dsp_SOURCE_DIR}/FormantFilter.h
        ${zyn.dsp_SOURCE_DIR}/SVFilter.h
        ${zyn.dsp_SOURCE_DIR}/Unison.h
    DESTINATION
        "include/zyn/dsp"
    )

install(
    EXPORT
        zyn.dsp
    FILE
        zyn.dsp-config.cmake
    DESTINATION
        "cmake"
    )
