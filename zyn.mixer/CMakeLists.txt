cmake_minimum_required(VERSION 3.8)

project(zyn.mixer)

add_library(zyn.mixer STATIC
    BankManager.cpp
    BankManager.h
    Track.cpp
    Track.h
    Meter.cpp
    Meter.h
    Microtonal.cpp
    Microtonal.h
    Mixer.cpp
    Mixer.h
    )

target_include_directories(zyn.mixer
    PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(zyn.mixer
    zyn.synth
    zyn.fx
    zyn.dsp
    zyn.common
    )

target_compile_features(zyn.mixer
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_range_for
        cxx_thread_local
    )

install(
    TARGETS
        zyn.mixer
    EXPORT
        zyn.mixer
    DESTINATION
        "lib"
    )

install(
    FILES
        ${zyn.mixer_SOURCE_DIR}/Bank.h
        ${zyn.mixer_SOURCE_DIR}/Microtonal.h
        ${zyn.mixer_SOURCE_DIR}/Mixer.h
        ${zyn.mixer_SOURCE_DIR}/Track.h
    DESTINATION
        "include/zyn/mixer"
    )

install(
    EXPORT
        zyn.mixer
    FILE
        zyn.mixer-config.cmake
    DESTINATION
        "cmake"
    )
